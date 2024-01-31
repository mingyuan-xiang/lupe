"""The front end interface of Lupe for enabling DNN on MSP430"""

# TODO: lupe can:
# + generate the model for msp430 from onnx/pytorch/tf
# + compiler & flash the model
# + open and shell for serial port

import argparse

def lupe_args():
    """Get the command line arguments for lupe
    
    Returns:
        Return an argparse instance
    """
    par = argparse.ArgumentParser()
    par.add_argument(
        '--port', type=str, default='/dev/cu.usbmodem1203', help='UART port'
    )
    par.add_argument('--baud', type=int, default=19200, help='UART baud rate')
    par.add_argument(
        '--model-name', type=str, default='BestModel', help='model name'
    )
    par.

    return par.parse_args()
