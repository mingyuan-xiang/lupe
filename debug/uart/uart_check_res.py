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

parser = argparse.ArgumentParser()
parser.add_argument(
    '--port', type=str, default='/dev/cu.usbmodem1203', help='UART port'
)
parser.add_argument('--baud', type=int, default=19200, help='UART baud rate')

models = {
    "DATE_LeNet" : DATE_LeNet()
}

parser.add_argument('--dataset', default='mnist', choices=['mnist'],
                    help='dataset (default: mnist)')
parser.add_argument('-a', '--arch', metavar='ARCH', default='DATE_LeNet',
                    choices=models.keys(),
                    help='model architecture: ' +
                        ' | '.join(models.keys()) +
                        ' (default: DATE_LeNet)')
parser.add_argument(
        "--idx", type=int, default=0,
        help="Set index of the input image(data) in the dataset for comparison"
    )
parser.add_argument(
    '--param-loc', '-p',
    default='./models/checkpoints/DATE_LeNet/model_best.pth',
    help='weights location (default: ./models/checkpoints/DATE_LeNet/model_best.pth)'
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

assert(len(l) == len(l_exp))

for i, x in enumerate(l):
    x_exp = l_exp[i].cpu().detach().numpy()
    # Convert to q15
    x_exp = (x_exp * (2 ** 15)).astype(np.int16)
    print("\n+++++++++++++++++++++++++++++++++++\n")
    print("\n============== Error ==============\n")
    print(((x - x_exp) ** 2).mean())
    print("\n============== Got ==============\n")
    print(x)
    print("\n============== Expected ==============\n")
    print(x_exp)
    print("\n+++++++++++++++++++++++++++++++++++\n")
