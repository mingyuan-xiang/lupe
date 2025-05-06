#!/usr/bin/env python3

"""The front end interface of Lupe for enabling DNN on MSP430"""

import argparse
from collections import OrderedDict
from colorama import Fore, Style
from enum import Enum, auto
import os
import pathlib
import json
import shutil
import threading
import queue

from jinja2 import Template
import onnx
from onnx import checker

from graph.graph import LupeGraph
from codegen.codegen import msp430gen
from debug.get_input import get_input
from calibration.calibration import calibration, update_calibration_config

class LupeMode(Enum):
    """Code generation mode"""
    NORMAL = auto()
    DEBUG = auto()
    CALIBRATING = auto()

def lupe_args():
    """Get the command line arguments for lupe
    
    Returns:
        Return an argparse instance
    """
    par = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    par.add_argument("mode", type=str, choices=[
        "code-gen", "compile", "flash", "print", "calibrate"],
        help="""Mode of operation:
        code-gen: Generate the C code for the model.
        compile: Compile the generated code using CMU Abstract maker.
        flash: Compile and flash the model to the MSP430.
        print: Print the the model.
        calibrate: calibrate the acceleration selection"""
    )
    par.add_argument(
        "--model-name", type=str, default="LeNet", help="Model name"
    )
    par.add_argument(
        "--model-path", type=str, default="./models/onnx/LeNet.onnx",
        help="Model path of the onnx representation"
    )
    par.add_argument(
        "--config", type=str, default="./configs/no_opt_lea.json",
        help="Optimization configuration file for the model"
    )
    par.add_argument(
        "--dataset-size", type=int, default=10000, help="Size of the dataset"
    )
    par.add_argument(
        "--timer", action=argparse.BooleanOptionalAction,
        default=True, help="Add timer to the code"
    )
    par.add_argument(
        "--output-path", type=str, help="Output path for the code"
    )
    par.add_argument(
        "--print-freq", type=int, default=10, help="Print frequency"
    )
    par.add_argument(
        "--loc", type=str, default="hi", choices=["hi", "lo"],
        help="Sections of the weights"
    )
    par.add_argument(
        "--clean", action=argparse.BooleanOptionalAction,
        default=True, help="Clean the output directory"
    )
    par.add_argument(
        "--debug", action=argparse.BooleanOptionalAction, default=False,
        help="Insert printing for input/output into the model"
    )
    par.add_argument(
        "--debug-random", action=argparse.BooleanOptionalAction, default=False,
        help="Generate random input for debugging"
    )
    par.add_argument(
        "--debug-random-seed", type=int, default=0,
        help="Seed for random input"
    )
    par.add_argument(
        "--debug-dataset", choices=[
            "MNIST", "CIFAR10", "vww", "sc", "fashion_mnist"],
        default="MNIST",
        help="Set the input buffer to be the first image(data) in the dataset"
    )
    par.add_argument(
        "--debug-idx", type=int, default=0,
        help="Set index of the input image(data) in the dataset for debugging"
    )
    par.add_argument(
        "--qf", type=int, default=2,
        help=("Set the bit width of the integer part of the fixed point " +
        "representation, e.g. pass `--qf 2` will have q2.13")
    )
    par.add_argument(
        "--qf-offset", type=int, default=1,
        help=("Set the fixed point format offset for weights")
    )
    par.add_argument(
        "--baud", type=int, default=19200, help="UART baud rate for calibration"
    )
    par.add_argument(
        "--port", type=str, default="/dev/cu.usbmodem1203",
        help="UART port for calibration"
    )
    par.add_argument(
        "--hifram-func", type=int, default=0,
        help=("Number of functions to be put on the HIFRAM. The model function"
        "will first be put onto the HIFRAM, and then layer functions.")
    )
    par.add_argument(
        "--max-dma-size", type=int, default=10000,
        help="Maximum DMA size (used for intermittent computing)"
    )
    par.add_argument(
        "--intermittent", action=argparse.BooleanOptionalAction, default=False,
        help="Add intermittent-safe support."
    )
    par.add_argument(
        "--intermittent-repeat", type=int, default=100,
        help="The number for repeating the experiments."
    )
    par.add_argument(
        "--intermittent-verify", action=argparse.BooleanOptionalAction, default=False,
        help=("Use software reset to verify the correctness of the computation."
        "It's not automatic. User need to manually copy the correct"
        "input output.")
    )
    par.add_argument(
        "--intermittent-bound", nargs='+', type=int, default=[328],
        help=("Upper and lower bound of software reset cycles for RNG."
        "If one number is given, we will constantly reset for that cycles.")
    )

    args = par.parse_args()

    if len(args.intermittent_bound) > 2:
        raise ValueError("intermittent-bound only accepts 1 or 2 inputs.")
    if (len(args.intermittent_bound) == 2 and
        args.intermittent_bound[0] > args.intermittent_bound[1]):
        raise ValueError("Lower bound is greater than upper bound for intermittent-bound.")

    return args

def load_opt_config(config):
    """Load the optimization configuration"""
    with open(config, "r", encoding="utf-8") as file:
        return OrderedDict(json.load(file))

def parse_opt_config(opt_config, name):
    """Parse the configuration file. Set default to False if not present."""
    opt_config["name"] = name

    if "adaptive_gen_mem" not in opt_config:
        opt_config["adaptive_gen_mem"] = False
    if "adaptive_gen_lea" not in opt_config:
        opt_config["adaptive_gen_lea"] = False
    if "lea_opt" not in opt_config:
        opt_config["lea_opt"] = False
    if "dma_opt" not in opt_config:
        opt_config["dma_opt"] = False
    if "global_mem_buffer" not in opt_config:
        opt_config["global_mem_buffer"] = False
    if "lea_size" not in opt_config:
        if opt_config["global_mem_buffer"]:
            opt_config["lea_size"] = 1600
        else:
            opt_config["lea_size"] = 400

    if opt_config["lea_size"] % 2:
        raise ValueError("lea_size has to be multiple of 2")

    if opt_config["adaptive_gen_mem"]:
        # The cutoff size for dma and loop copy on MSP430
        opt_config["adaptive_gen_mem_size"] = 11

    return opt_config

def _banner_print(s):
    print(Fore.GREEN +
        '\n*******************************************************************')
    print(Fore.GREEN + s)
    print(Fore.GREEN +
        '\n*******************************************************************'
        + Fore.WHITE + Style.RESET_ALL)

parent = pathlib.Path(__file__).parent.resolve()

def _get_configs(args):
    # Read the optimization configuration
    config = {}
    if os.path.isfile(args.config):
        config = load_opt_config(args.config)

    name = os.path.basename(os.path.normpath(args.config))
    # remove the json part
    name = name.split(".")[0]

    parse_opt_config(config, name)

    return config

def _get_paths(args, mode):
    if mode == LupeMode.CALIBRATING:
        dir_name = args.model_name + "_calibration"

        out_path = os.path.join(parent, "apps", dir_name)

        if os.path.exists(out_path):
            shutil.rmtree(out_path)
    else:
        dir_name = args.model_name

        if args.output_path is None:
            out_path = os.path.join(parent, "apps", args.model_name)
        else:
            out_path = args.output_path

        if args.clean and os.path.exists(out_path):
            shutil.rmtree(out_path)

    print(f"Writing to {out_path}, got optimization flags:")

    if not os.path.exists(out_path):
        os.makedirs(out_path)

    return out_path, dir_name

def _get_lupe_graph(args, config, out_path, dir_name):
    # Load onnx model
    if os.path.isfile(args.model_path):
        model = onnx.load(args.model_path)
        checker.check_model(model)

        graph = LupeGraph(
            dir_name, model, out_path, config, qf_offset=args.qf_offset
        )
    else:
        raise FileNotFoundError(
            f"The model file {args.model_path} does not exist"
        )

    return graph

def _generate(args, mode, graph, config, out_path, dir_name):
    # Read the acceleration configuration
    acc_config = None
    if (config["adaptive_gen_lea"] and
        mode in (LupeMode.NORMAL, LupeMode.DEBUG)):
        cal_path = os.path.join(
            parent, "calibration", args.model_name + ".json")
        if os.path.isfile(cal_path):
            acc_config = load_opt_config(cal_path)

        if acc_config is None or 'opt_config' not in acc_config:
            raise ValueError(
                "Optimization flags cannot be found in calibration"
                " configuration!"
            )
        if acc_config['opt_config'] != args.config:
            raise ValueError(
                "Optimization flags for calibration and code generation"
                " don't match!"
            )

        # Set acceleration method
        graph.set_acceleration(acc_config)

    cal = False
    if mode == LupeMode.CALIBRATING:
        cal = True

    generator = msp430gen()(
        out_path, config, graph, args.qf, add_timer=args.timer,
        calibration=cal
    )

    if mode == LupeMode.DEBUG or args.debug_random:
        input_arr, label = get_input(
            args.debug_dataset, args.debug_idx, args.debug_random,
            args.debug_random_seed
        )
        generator.setup_debug_info(
            input_arr / (2 ** args.qf), label, args.debug
        )

    # Print the optimization configurations
    generator.print_config()


    if  mode != LupeMode.CALIBRATING:
        intermittent_args = None
        if args.intermittent:
            intermittent_args = {
                "repeat" : args.intermittent_repeat,
                "bounds" : args.intermittent_bound,
                "verify" : args.intermittent_verify,
            }

        generator.gen(
            args.dataset_size, args.max_dma_size, args.print_freq,
            calibration=cal, intermittent=args.intermittent,
            intermittent_args=intermittent_args, hifram_func=args.hifram_func
        )
    else:
        generator.gen(
            args.dataset_size, args.max_dma_size, args.print_freq,
            calibration=cal, hifram_func=args.hifram_func
        )

    # Generate the outer Makefile for the maker
    template_path = os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "makefile.jinja"
    )
    with open(template_path, "r", encoding="utf-8") as file:
        template = file.read()
        j_template = Template(template)
        code_str = j_template.render({"model_name" : dir_name})

    file_path = os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "Makefile"
    )
    if os.path.isfile(file_path):
        os.remove(file_path)
    with open(os.path.join(file_path), "w", encoding="utf-8") as file:
        file.write(code_str)

def main():
    """The main function"""
    args = lupe_args()

    if args.mode == "print":
        # Load onnx model
        if os.path.isfile(args.model_path):
            model = onnx.load(args.model_path)
            checker.check_model(model)

            graph = LupeGraph(args.model_name, model, "", {})
            graph.print()
    elif args.mode == "code-gen":
        mode = LupeMode.NORMAL
        if args.debug:
            mode = LupeMode.DEBUG
        config = _get_configs(args)
        out_path, dir_name = _get_paths(args, mode)
        graph = _get_lupe_graph(args, config, out_path, dir_name)
        _generate(args, mode, graph, config, out_path, dir_name)
    elif args.mode == "compile":
        os.system(f"make apps/{args.model_name}/bld/gcc/all")
    elif args.mode == "flash":
        os.system(f"make apps/{args.model_name}/bld/gcc/prog")
    elif args.mode == "calibrate":
        _banner_print('Start the calibration for adaptive layer generation.')

        _banner_print('Generate calibration code')
        mode = LupeMode.CALIBRATING
        config = _get_configs(args)
        out_path, dir_name = _get_paths(args, mode)
        graph = _get_lupe_graph(args, config, out_path, dir_name)

        acc_dict = {}

        _banner_print('Start the calibration in the background')

        while graph.need_calibration():
            _generate(args, mode, graph, config, out_path, dir_name)

            result_queue = queue.Queue()
            cal_thread = threading.Thread(
                target=calibration,
                args=(args.baud, args.port, result_queue)
            )
            cal_thread.daemon = True
            cal_thread.start()

            _banner_print('Compile and flash calibration code')
            dir_name = args.model_name + "_calibration"
            os.system(f"make apps/{dir_name}/bld/gcc/prog")

            _banner_print('Waiting for calibration results...')
            while True:
                if not result_queue.empty():
                    break

            cal_thread.join()

            d = result_queue.get()

            update_calibration_config(acc_dict, d)

            graph.update_calibration_idx()

        _banner_print('Write out the calibration configurations')
        acc_dict['opt_config'] = args.config

        # create calibration directory if not existed
        parent = pathlib.Path(__file__).parent.resolve()
        cal_dir = os.path.join(parent, "calibration")
        if not os.path.exists(cal_dir):
            os.makedirs(cal_dir)
        cal_file = os.path.join(cal_dir, args.model_name + ".json")
        with open(cal_file, "w", encoding="utf-8") as file:
            json.dump(acc_dict, file, indent=4)

if __name__ == "__main__":
    main()
