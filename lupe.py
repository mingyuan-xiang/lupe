#!/usr/bin/env python3

"""The front end interface of Lupe for enabling DNN on MSP430"""

import argparse
from collections import OrderedDict
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
from calibration.calibration import calibration

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
        "--model-name", type=str, default="DATE_LeNet", help="Model name"
    )
    par.add_argument(
        "--model-path", type=str, default="./models/onnx/DATE_LeNet.onnx",
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
        "--debug-dataset", choices=["MNIST", "CIFAR10"], default="MNIST",
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
        "--baud", type=int, default=19200, help="UART baud rate for calibration"
    )
    par.add_argument(
        "--port", type=str, default="/dev/cu.usbmodem1203",
        help="UART port for calibration"
    )

    return par.parse_args()

def load_opt_config(config):
    """Load the optimization configuration"""
    with open(config, "r", encoding="utf-8") as file:
        return OrderedDict(json.load(file))

def parse_opt_config(opt_config):
    """Parse the configuration file. Set default to False if not present."""
    if "enhanced_acc" not in opt_config:
        opt_config["enhanced_acc"] = False
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
    print('\n*****************************************************************')
    print(s)
    print('*****************************************************************\n')

def _generate(args, mode):
    # Load onnx model
    if os.path.isfile(args.model_path):
        model = onnx.load(args.model_path)
        checker.check_model(model)

        if mode == LupeMode.CALIBRATING:
            dir_name = args.model_name + "_calibration"

            parent = pathlib.Path(__file__).parent.resolve()
            out_path = os.path.join(parent, "apps", dir_name)

            if os.path.exists(out_path):
                shutil.rmtree(out_path)
        else:
            dir_name = args.model_name

            parent = pathlib.Path(__file__).parent.resolve()
            if args.output_path is None:
                out_path = os.path.join(parent, "apps", args.model_name)
            else:
                out_path = args.output_path

            if args.clean and os.path.exists(out_path):
                shutil.rmtree(out_path)

        print(f"Writing to {out_path}, got optimization flags:")

        if not os.path.exists(out_path):
            os.makedirs(out_path)

        # Read the optimization configuration
        config = {}
        if os.path.isfile(args.config):
            config = load_opt_config(args.config)

        parse_opt_config(config)

        graph = LupeGraph(dir_name, model, out_path, config)

        # Read the acceleration configuration
        acc_config = None
        if (config["adaptive_gen_lea"] and
            mode in (LupeMode.NORMAL, LupeMode.DEBUG)):
            cal_path = os.path.join(
                parent, "calibration", args.model_name + ".json")
            if os.path.isfile(cal_path):
                acc_config = load_opt_config(cal_path)

            if 'opt_config' not in acc_config:
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

        if mode == LupeMode.DEBUG:
            input_arr, label = get_input(args.debug_dataset, args.debug_idx)
            generator.setup_debug_info(input_arr / (2 ** args.qf), label)

        # Print the optimization configurations
        generator.print_config()

        generator.gen(
            dir_name, args.dataset_size, args.print_freq, calibration=cal
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
    else:
        raise FileNotFoundError(
            f"The model file {args.model_path} does not exist"
        )

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
        _generate(args, mode)
    elif args.mode == "compile":
        os.system(f"make apps/{args.model_name}/bld/gcc/all")
    elif args.mode == "flash":
        os.system(f"make apps/{args.model_name}/bld/gcc/prog")
    elif args.mode == "calibrate":
        _banner_print('Start the calibration for adaptive layer generation.')

        _banner_print('Generate calibration code')
        _generate(args, LupeMode.CALIBRATING)

        _banner_print('Start the calibration in the background')

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

        _banner_print('Write out the calibration configurations')
        acc_dict = result_queue.get()

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
