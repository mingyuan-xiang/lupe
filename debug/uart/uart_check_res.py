"""send input images for MNIST"""

import os
import sys
import argparse
import importlib
import inspect

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
from DATE_LeNet import DATE_LeNet
from DATE_LeNet_quan import DATE_LeNet_quan

parser = argparse.ArgumentParser()
parser.add_argument(
    '--port', type=str, default='/dev/cu.usbmodem1203', help='UART port'
)
parser.add_argument('--baud', type=int, default=19200, help='UART baud rate')

models = {
    "DATE_LeNet" : DATE_LeNet(),
    "DATE_LeNet_quan" : DATE_LeNet_quan(),
}

parser.add_argument('--dataset', default='mnist', choices=['mnist'],
                    help='dataset (default: mnist)')
parser.add_argument('-a', '--arch', metavar='ARCH', default='DATE_LeNet_quan',
                    choices=models.keys(),
                    help='model architecture: ' +
                        ' | '.join(models.keys()) +
                        ' (default: DATE_LeNet_quan)')
parser.add_argument(
        "--idx", type=int, default=0,
        help="Set index of the input image(data) in the dataset for comparison"
    )
parser.add_argument(
    '--param-loc', '-p',
    default='./models/checkpoints/DATE_LeNet_quan/model_best.pth',
    help='weights location (default: ./models/checkpoints/DATE_LeNet_quan/model_best.pth)'
)
parser.add_argument(
    "--qf", type=int, default=2,
    help=("Set the bit width of the integer part of the fixed point " +
    "representation, e.g. pass `--qf 2` will have q2.13")
)

args = parser.parse_args()
par = sync_reader(args.port, args.baud)

np.set_printoptions(linewidth=np.inf, threshold=np.inf)

l = []
while True:
    msg = par.get_msg()
    if isinstance(msg, np.ndarray):
        l.append(msg)
    if isinstance(msg, str) and msg == UARTIO_END_PRINT_STR:
        break

image, label = get_input(args.dataset, args.idx)
image = torch.from_numpy(image)

model = models[args.arch]
model.load_state_dict(torch.load(args.param_loc)['state_dict'])

x, l_exp = model(image)

names = model.names

assert(len(l) == len(l_exp) and len(names) == len(l))

max_i16 = np.iinfo(np.int16).max

for i, x in enumerate(l):
    x_exp = l_exp[i].cpu().detach().numpy()
    # Convert to q15
    x_exp = x_exp * (2 ** (15 - args.qf))
    # Convert the overflow numbers to 0
    x_exp = x_exp * (x_exp < max_i16 - 1)
    x_exp = x_exp.astype(np.int16)
    print(f"\n++++++++++++++ {names[i]} ++++++++++++++\n")
    print("\n============== Error ==============\n")
    print(np.abs(x - x_exp))
    print("\n============== Got ==============\n")
    print(x)
    print("\n============== Expected ==============\n")
    print(x_exp)
    print("\n+++++++++++++++++++++++++++++++++++\n")
