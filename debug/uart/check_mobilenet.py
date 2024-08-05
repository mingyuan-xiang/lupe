"""
Load the ONNX model and check the results of the specific layer of ResNet.
The ONNX adds layer fusion, which makes it hard to compare results.
So, I wrote another script for debugging.
"""

import os
import sys
import argparse

import numpy as np
import torch
import torch.nn.functional as F
import onnx

cur_dir = os.path.dirname(os.path.abspath(__file__))
root = os.path.dirname(os.path.dirname(cur_dir))
import_path = os.path.join(root, "scripts", "uart_commute")
sys.path.append(import_path)
sys.path.append(os.path.dirname(cur_dir))

from uart_dump import sync_reader, UARTIO_END_PRINT_STR
from get_input import get_input

parser = argparse.ArgumentParser()
parser.add_argument(
    '--port', type=str, default='/dev/cu.usbmodem1203', help='UART port'
)
parser.add_argument('--baud', type=int, default=19200, help='UART baud rate')
parser.add_argument(
        "--idx", type=int, default=0,
        help="Set index of the input image(data) in the dataset for comparison"
)
parser.add_argument(
    '--param-loc', '-p',
    default='./models/onnx/MobileNetV2.onnx'
)
parser.add_argument(
    "--qf", type=int, default=2,
    help=("Set the bit width of the integer part of the fixed point " +
    "representation, e.g. pass `--qf 2` will have q2.13")
)

args = parser.parse_args()

np.set_printoptions(linewidth=np.inf, threshold=np.inf, precision=3)

image, label = get_input('vww', args.idx)
image = torch.from_numpy(image)
int16_min = np.iinfo(np.int16).min / (2 ** (15 - args.qf))
int16_max = np.iinfo(np.int16).max / (2 ** (15 - args.qf))
image = np.clip(image, int16_min, int16_max)

model = onnx.load(args.param_loc)
weights = model.graph.initializer


def get_weights(w_name, b_name):
    weight = None
    bias = None
    for w in weights:
        if w.name == w_name:
            weight = onnx.numpy_helper.to_array(w)
            weight = torch.from_numpy(weight)
        if w.name == b_name:
            bias = onnx.numpy_helper.to_array(w)
            bias = torch.from_numpy(bias)

    return weight, bias

weight, bias = get_weights('onnx::Conv_199', 'onnx::Conv_200')
res = F.conv2d(image, weight, bias=bias, padding=1, stride=2)
res = F.relu6(res)
weight, bias = get_weights('onnx::Conv_202', 'onnx::Conv_203')
res = F.conv2d(res, weight, bias=bias, padding=1, groups=8, stride=2)
res = F.relu6(res)
weight, bias = get_weights('onnx::Conv_205', 'onnx::Conv_206')
res = F.conv2d(res, weight, bias=bias)
# res = res + x
# res = F.relu6(res)
# weight, bias = get_weights('onnx::Conv_132', 'onnx::Conv_133')
# x = F.conv2d(res, weight, bias=bias, stride=2)
# weight, bias = get_weights('onnx::Conv_126', 'onnx::Conv_127')
# res = F.conv2d(res, weight, bias=bias, stride=2, padding=1)
# res = F.relu6(res)
# weight, bias = get_weights('onnx::Conv_129', 'onnx::Conv_130')
# res = F.conv2d(res, weight, bias=bias, padding=1)
# res = res + x
# res = F.relu6(res)
# weight, bias = get_weights('onnx::Conv_141', 'onnx::Conv_142')
# x = F.conv2d(res, weight, bias=bias, stride=2)
# weight, bias = get_weights('onnx::Conv_135', 'onnx::Conv_136')
# res = F.conv2d(res, weight, bias=bias, stride=2, padding=1)
# res = F.relu6(res)
# weight, bias = get_weights('onnx::Conv_138', 'onnx::Conv_139')
# res = F.conv2d(res, weight, bias=bias, padding=1)
# res = res + x
# res = F.relu6(res)

x_exp = res.cpu().detach().numpy()
x_exp = x_exp * (2 ** (15 - args.qf))

print("Done inference!")

par = sync_reader(args.port, args.baud)


l = []
while True:
    msg = par.get_msg()
    if isinstance(msg, np.ndarray):
        l.append(msg)
    else:
        print(msg)
    if isinstance(msg, str) and msg == UARTIO_END_PRINT_STR:
        break

x = l[0]
x_exp = x_exp.astype(np.int16)
print(f"\n++++++++++++++ conv1 ++++++++++++++\n")
print("\n============== Error ==============\n")
print((np.abs((x - x_exp).astype('float') / x) * 100).astype(np.int16))
print("\n============== Got ==============\n")
print(x)
print("\n============== Expected ==============\n")
print(x_exp)
print("\n+++++++++++++++++++++++++++++++++++\n")
