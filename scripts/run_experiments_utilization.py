import numpy as np

from colorama import Fore, Style
import threading
import subprocess
import argparse
import sys
import os
sys.path.append("scripts/uart_commute")
from uart_dump import sync_reader

PRINT_CNT = 10

config_list = [
    'no_opt',
    'dma_lea_opt_adaptive_buffer_mem',
]

model_list = [
    ('ResNet3', (4, 'CIFAR10')),
    ('MobileNetV2', (4, 'vww')),
    ('DS_CNN', (5, 'sc')),
    ('LeNet', (5, 'MNIST')),
    ('MLPClassifier', (5, 'fashion_mnist')),
]

def _banner_print(s):
    print(Fore.GREEN +
        '\n*******************************************************************')
    print(Fore.GREEN + s)
    print(Fore.GREEN +
        '\n*******************************************************************'
        + Fore.WHITE + Style.RESET_ALL)

def get_args():
    par = argparse.ArgumentParser()
    par.add_argument(
        '--port', type=str, default='/dev/cu.usbmodem1203', help='UART port'
    )
    par.add_argument('--baud', type=int, default=19200, help='UART baud rate')
    par.add_argument('--log-prefix', type=str, default='fir', help='log file prefix')

    args = par.parse_args()

    return args

args = get_args()

def run_lupe(model, dataset, qf, config):
    onnx_path = os.path.join("models", "onnx", model + ".onnx")

    config_path = os.path.join("configs", config + ".json")

    subprocess.run([
        "./lupe.py", "code-gen", "--debug-random", "--debug-dataset", dataset,
        "--model-name", model, "--model-path", onnx_path, "--qf", str(qf),
        "--config", config_path,
    ], check=False)

filename = "logs"
os.makedirs(filename, exist_ok=True)
filename = os.path.join(filename, args.log_prefix + ".log")
if os.path.isfile(filename):
    os.remove(filename)

f = open(filename, "w")

def enable_uart(baud, port, config):
    par = sync_reader(port, baud)

    # read start string
    while True:
        msg = par.get_msg()
        if msg is not None and "Test" in msg:
            break

    print("=============== Start computing ===============")
    f.write(msg + f'({config})'+ "\n")

    while True:
        msg = par.get_msg()
        print(msg)
        f.write(msg + "\n")

        if 'Finish' in msg:
            f.flush()
            break

    par.close()

for m in model_list:
    for c in config_list:
        _banner_print(f"Generate Code for {m[0]}")
        run_lupe(m[0], m[1][1], m[1][0], c)

        uart_thread = threading.Thread(
            target=enable_uart,
            args=(args.baud, args.port, c)
        )
        uart_thread.daemon = True

        _banner_print('Compile and flash code')
        os.system(f"make apps/{m[0]}/bld/gcc/prog")

        uart_thread.start()

        uart_thread.join()

f.close()