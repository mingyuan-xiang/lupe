#!/usr/bin/env python3

"""The front end interface of Lupe for enabling DNN on MSP430"""

import argparse
import os
import pathlib
import json

import onnx
from onnx import checker

from graph.graph import LupeGraph
from codegen.codegen import msp430gen

def lupe_args():
    """Get the command line arguments for lupe
    
    Returns:
        Return an argparse instance
    """
    par = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter)
    par.add_argument("mode", type=str, choices=["code-gen", "flash", "print"],
        help="""Mode of operation:
        code-gen: Generate the C code for the model.
        flash: Compile and flash the model to the MSP430.
        print: Print the the model."""
    )
    par.add_argument(
        "--model-name", type=str, default="LeNet", help="Model name"
    )
    par.add_argument(
        "--model-path", type=str, default="./models/onnx/LeNet.onnx",
        help="Model path of the onnx representation"
    )
    par.add_argument(
        "--config", type=str, default="./configs/no_opt.json",
        help="Optimization configuration file for the model"
    )
    par.add_argument(
        "--dataset-size", type=int, default=100, help="Size of the dataset"
    )
    par.add_argument(
        "--timer", type=bool, default=True, help="Add timer to the code"
    )
    par.add_argument(
        "--output-path", type=str, help="Output path for the code"
    )
    par.add_argument(
        "--print-freq", type=int, default=100, help="Print frequency"
    )
    par.add_argument(
        "--loc", type=str, default="hi", choices=["hi", "lo"],
        help="Sections of the weights"
    )

    return par.parse_args()

def load_opt_config(config):
    """Load the optimization configuration"""
    with open(config, "r", encoding="utf-8") as file:
        opt_config = json.load(file)
        return opt_config

def main():
    """The main function"""
    args = lupe_args()

    if args.mode == "print":
        # Load onnx model
        if os.path.isfile(args.model_path):
            model = onnx.load(args.model_path)
            checker.check_model(model)

            graph = LupeGraph(args.model_name, model, "")
            graph.print()
    elif args.mode == "code-gen":
        # Load onnx model
        if os.path.isfile(args.model_path):
            model = onnx.load(args.model_path)
            checker.check_model(model)

            parent = pathlib.Path(__file__).parent.resolve()
            if args.output_path is None:
                out_path = os.path.join(parent, "apps", args.model_name)
            else:
                out_path = args.output_path
            # Create the directory if it does not exist
            if not os.path.exists(out_path):
                os.makedirs(out_path)
            # Create the include directory
            if not os.path.exists(os.path.join(out_path, "include")):
                os.makedirs(os.path.join(out_path, "include"))

            graph = LupeGraph(args.model_name, model, out_path)

            # Read the optimization configuration
            if os.path.isfile(args.config):
                config = load_opt_config(args.config)

            print(config)

            generator = msp430gen()(
                out_path, config, graph, add_timer=args.timer
            )
            generator.gen(args.model_name, args.dataset_size, args.print_freq)
    elif args.mode == "flash":
        print("Flash")
        # TODO: configure environment here

if __name__ == "__main__":
    main()
