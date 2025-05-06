from torchvision import datasets, transforms

import numpy as np

from colorama import Fore, Style
import threading
import subprocess
import argparse
import sys
import os
sys.path.append("scripts/uart_commute")
from uart_dump import sync_reader

import pyvww

sys.path.append("models/dataset")
from speech_commands import SpeechCommands

PRINT_CNT = 10

data_root_dir = '/home/myxiang/Documents/data'

config_list = [
    'no_opt',
    'dma',
    'dma_lea_opt',
    'dma_lea_opt_adaptive_buffer',
    'dma_lea_opt_adaptive_buffer_mem',
    'dma_lea_opt_adaptive_buffer_mem_acc',
]

def _banner_print(s):
    print(Fore.GREEN +
        '\n*******************************************************************')
    print(Fore.GREEN + s)
    print(Fore.GREEN +
        '\n*******************************************************************'
        + Fore.WHITE + Style.RESET_ALL)

def get_dataset(dataset):
    """get dataset"""
    if dataset.lower() == "mnist":
        d = datasets.MNIST(data_root_dir, train=False,
            transform=transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize((0.5,), (0.5,))
        ]), download=True)
    elif dataset.lower() == "cifar10":
        d = datasets.CIFAR10(
            root=data_root_dir, train=False, transform=transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize(mean=[0.485, 0.456, 0.406],
            std=[0.229, 0.224, 0.225])
        ]), download=True)
    elif dataset.lower() == "vww":
        normalize = transforms.Normalize(mean=[0.485, 0.456, 0.406],
            std=[0.229, 0.224, 0.225])
        d = pyvww.pytorch.VisualWakeWordsClassification(
            root=os.path.join(data_root_dir, 'vww/all2017/'),
            annFile=os.path.join(
                data_root_dir, 'vww/annotations/instances_val.json'),
            transform=transforms.Compose([
                transforms.Resize((80, 80)),
                transforms.ToTensor(),
                normalize,
            ]))
    elif dataset.lower() == "sc":
        d = SpeechCommands(
            os.path.join(data_root_dir, 'SpeechCommands'), 'testing')
    elif dataset.lower() == "fashion_mnist":
        transform = transforms.Compose(
            [transforms.ToTensor(),
            transforms.Normalize((0.5,), (0.5,))])
        d = datasets.FashionMNIST(
            data_root_dir, train=False, transform=transform, download=True)
    else:
        raise NotImplementedError(f"{dataset} is not supported")

    return d

# pack the image to a 1D 16-bit array
def parse_img(img, qf):
    img = img.numpy()
    int16_min = np.iinfo(np.int16).min / (2 ** (15 - qf))
    int16_max = np.iinfo(np.int16).max / (2 ** (15 - qf))
    img = np.clip(img, int16_min, int16_max)
    img = img.flatten() * (2 ** (15 - qf))
    img = img.astype(np.int16)
    return img

def get_args():
    par = argparse.ArgumentParser()
    par.add_argument(
        '--port', type=str, default='/dev/ttyACM3', help='UART port'
    )
    par.add_argument('--baud', type=int, default=19200, help='UART baud rate')
    par.add_argument('--model', default='LeNet', help='model name')
    par.add_argument("--model-path", type=str, help="Model path of the onnx representation")
    par.add_argument('--dataset', help='dataset')
    par.add_argument('--qf', default=2, help=("Set the bit width of the integer part of the fixed point " +
        "representation, e.g. pass `--qf 2` will have q2.13"))

    args = par.parse_args()

    return args

args = get_args()

def run_lupe(model, onnx_path, dataset_size, qf, config, mode):
    if onnx_path == None:
        onnx_path = os.path.join("models", "onnx", model + ".onnx")

    config_path = os.path.join("configs", config + ".json")

    subprocess.run([
        "./lupe.py", mode, "--model-name", model,
        "--model-path", onnx_path, "--qf", str(qf), "--config", config_path,
        "--dataset-size", str(dataset_size)
    ], check=False)

def enable_uart(baud, port, mode_name, config, qf, print_cnt, dataset):
    filename = "logs"
    os.makedirs(filename, exist_ok=True)
    filename = os.path.join(filename, mode_name + "_" + config + ".log")
    if os.path.isfile(filename):
        os.remove(filename)

    f = open(filename, "w")

    par = sync_reader(port, baud)

    # read start string
    while True:
        msg = par.get_msg()
        if msg is not None and "Test" in msg:
            break

    print("=============== Start computing ===============")
    cnt = 0
    print(f"qf={qf}")

    # Access each image as a matrix
    for idx, (image, label) in enumerate(dataset):
        img_arr = parse_img(image, qf)
        par.put_arr(img_arr)

        l = int(par.get_msg())

        cnt += (l == label)

        msg = (f"Image {idx}, true label: {label}, predicted label: {l}, " +
            f"accumulative accuracy: {cnt/ (idx + 1)}")
        print(msg)
        f.write(msg + "\n")

        if (idx % print_cnt == (print_cnt - 1)) or (idx == len(dataset) - 1):
            # receive the time
            msg = par.get_msg()
            print(msg)
            f.write(msg + "\n")
            f.flush()

    # receive end string
    print(par.get_msg())

    par.close()
    f.close()

d = get_dataset(args.dataset)

for c in config_list:
    if "acc" in c:
        _banner_print("Calibrate Accelerations")
        run_lupe(
            args.model, args.model_path, len(d), args.qf, c, mode="calibrate")

    _banner_print(f"Generate Code for {args.model}")
    run_lupe(args.model, args.model_path, len(d), args.qf, c, mode="code-gen")

    uart_thread = threading.Thread(
        target=enable_uart,
        args=(args.baud, args.port, args.model, c, int(args.qf), PRINT_CNT, d)
    )
    uart_thread.daemon = True
    uart_thread.start()

    _banner_print('Compile and flash code')
    os.system(f"make apps/{args.model}/bld/gcc/prog")

    uart_thread.join()
