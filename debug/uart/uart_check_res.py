"""send input images for MNIST"""

import os
import sys
import argparse

import numpy as np
import torch

cur_dir = os.path.dirname(os.path.abspath(__file__))
root = os.path.dirname(os.path.dirname(cur_dir))
import_path = os.path.join(root, "scripts", "uart_commute")
sys.path.append(import_path)
sys.path.append(os.path.dirname(cur_dir))
model_path = os.path.join(os.path.dirname(cur_dir), 'models')
sys.path.append(model_path)

from uart_dump import sync_reader, UARTIO_END_PRINT_STR
from get_input import get_input
from LeNet import LeNet
from ResNet3 import ResNet3

parser = argparse.ArgumentParser()
parser.add_argument(
    '--port', type=str, default='/dev/cu.usbmodem1203', help='UART port'
)
parser.add_argument('--baud', type=int, default=19200, help='UART baud rate')

parser.add_argument('--dataset', default='mnist', choices=['mnist', 'cifar10'],
                    help='dataset (default: mnist, cifar10)')
parser.add_argument('-a', '--arch', metavar='ARCH', default='LeNet')

parser.add_argument(
        "--idx", type=int, default=0,
        help="Set index of the input image(data) in the dataset for comparison"
    )
parser.add_argument(
    '--param-loc', '-p',
    default='./models/checkpoints/LeNet/model_best.pth',
    help='weights location (default: ./models/checkpoints/DATE_LeNet_quan/model_best.pth)'
)
parser.add_argument(
    "--qf", type=int, default=2,
    help=("Set the bit width of the integer part of the fixed point " +
    "representation, e.g. pass `--qf 2` will have q2.13")
)

args = parser.parse_args()
par = sync_reader(args.port, args.baud)

models = {
    "LeNet" : LeNet(),
    "ResNet3" : ResNet3(),
}

np.set_printoptions(linewidth=np.inf, threshold=np.inf, precision=3)

image, label = get_input(args.dataset, args.idx)
image = torch.from_numpy(image)

model = models[args.arch]
model.load_state_dict(torch.load(args.param_loc, map_location=torch.device('cpu'))['state_dict'])

x, l_exp = model(image)

print("Done inference!")

names = model.names

l = []
while True:
    msg = par.get_msg()
    if isinstance(msg, np.ndarray):
        l.append(msg)
    else:
        print(msg)
    if isinstance(msg, str) and msg == UARTIO_END_PRINT_STR:
        break

assert(len(l) == len(l_exp) and len(names) == len(l))

max_i16 = np.iinfo(np.int16).max
min_i16 = np.iinfo(np.int16).min

for i, x in enumerate(l):
    x_exp = l_exp[i].cpu().detach().numpy()
    # Convert to q15
    x_exp = x_exp * (2 ** (15 - args.qf))
    # Convert the overflow and underflow numbers to max_i16 and min_i16
    x_exp_64 = x_exp.astype(np.int64)
    x_exp = (
        x_exp * (x_exp < max_i16) +
        max_i16 * np.ones_like(x_exp) * (x_exp >= max_i16)
    )
    x_exp = (
        x_exp * (x_exp > min_i16) +
        min_i16 * np.ones_like(x_exp) * (x_exp <= min_i16)
    )
    x_exp = x_exp.astype(np.int16)
    print(f"\n++++++++++++++ {names[i]} ++++++++++++++\n")
    print("\n============== Error ==============\n")
    print((np.abs((x - x_exp).astype('float') / x) * 100).astype(np.int16))
    print("\n============== Got ==============\n")
    print(x)
    print("\n============== Expected ==============\n")
    print(x_exp)
    # print("\n============== Expected (int64) ==============\n")
    # print(x_exp_64)
    print("\n+++++++++++++++++++++++++++++++++++\n")
