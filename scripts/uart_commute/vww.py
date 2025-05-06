"""send input images for CIFAR10"""

from torchvision import datasets, transforms

import numpy as np

import argparse
import sys
import json
import os
import pyvww

sys.path.append("scripts/uart_commute")
from uart_dump import sync_reader

PRINT_CNT = 10

normalize = transforms.Normalize(mean=[0.485, 0.456, 0.406],
    std=[0.229, 0.224, 0.225])
vww = pyvww.pytorch.VisualWakeWordsClassification(
    root='models/data/vww/all2017/',
    annFile='models/data/vww/annotations/instances_val.json',
    transform=transforms.Compose([
        transforms.Resize((80, 80)),
        transforms.ToTensor(),
        normalize,
    ]))

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
        '--port', type=str, default='/dev/cu.usbmodem1203', help='UART port'
    )
    par.add_argument('--baud', type=int, default=19200, help='UART baud rate')
    par.add_argument('-a', '--arch', default='LeNet_quan', help='model name')
    par.add_argument('--qf', default=2, help=("Set the bit width of the integer part of the fixed point " +
        "representation, e.g. pass `--qf 2` will have q2.13"))
    par.add_argument('--config', default=None, type=str, metavar='PATH',
                    help='path to the JSON configuration (default: none)')

    args = par.parse_args()

    if args.config is not None:
        dir_path = os.path.dirname(os.path.abspath(__file__))
        config_path = os.path.join(dir_path, "configs", args.config)
        with open(config_path, 'r') as f:
            args_from_json = json.load(f)

        # Use the dictionary to set the arguments
        for key, value in args_from_json.items():
            if hasattr(args, key):
                setattr(args, key, value)
    return args

args = get_args()

filename = "logs"
os.makedirs(filename, exist_ok=True)
filename = os.path.join(filename, args.config + ".log")
if os.path.isfile(filename):
    os.remove(filename)

f = open(filename, "w")

par = sync_reader(args.port, args.baud)

# read start string
while True:
    msg = par.get_msg()
    if msg is not None and "Test" in msg:
        break

print("=============== Start computing ===============")

cnt = 0

print(f"qf={args.qf}")

# Access each image as a matrix
for idx, (image, label) in enumerate(vww):
    img_arr = parse_img(image, args.qf)
    par.put_arr(img_arr)

    l = int(par.get_msg())

    cnt += (l == label)

    msg = (f"Image {idx}, true label: {label}, predicted label: {l}, " +
        f"accumulative accuracy: {cnt/ (idx + 1)}")
    print(msg)
    f.write(msg + "\n")

    if (idx % PRINT_CNT == (PRINT_CNT - 1)) or (idx == len(vww) - 1):
        # receive the time
        msg = par.get_msg()
        print(msg)
        f.write(msg + "\n")
        f.flush()

# receive end string
print(par.get_msg())

par.close()
f.close()
