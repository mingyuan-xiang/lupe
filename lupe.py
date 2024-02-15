#!/usr/bin/env python3

"""The front end interface of Lupe for enabling DNN on MSP430"""

import argparse
import os
import pathlib

import onnx
from onnx import checker

from graph.graph import LupeGraph

def lupe_args():
    """Get the command line arguments for lupe
    
    Returns:
        Return an argparse instance
    """
    par = argparse.ArgumentParser()
    par.add_argument("mode", type=str, choices=["code-gen", "flash", "print"],
        help="""Mode of operation:
        code-gen: Generate the C code for the model
        flash: Compile and flash the model to the MSP430."""
    )
    par.add_argument(
        "--model-name", type=str, default="LeNet", help="Model name"
    )
    par.add_argument(
        "--model-path", type=str, default="./models/onnx/LeNet.onnx",
        help="Model path of the onnx representation"
    )
    par.add_argument(
        "--config", type=str, default="./config/no_opt.json",
        help="Optimization configuration file for the model"
    )

    return par.parse_args()

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
            out_path = os.path.join(parent, "apps", args.model_name)
            graph = LupeGraph(args.model_name, model, out_path)
    elif args.mode == "flash":
        print("Flash")
        # TODO: configure environment here

if __name__ == "__main__":
    main()
