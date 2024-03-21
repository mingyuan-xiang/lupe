"""send input images for MNIST"""

import os
import sys

import numpy as np

cur_dir = os.path.dirname(os.path.abspath(__file__))
root = os.path.dirname(os.path.dirname(cur_dir))
import_path = os.path.join(root, "scripts", "uart_commute")
sys.path.append(import_path)
from uart_dump import sync_reader, get_args

args = get_args()
par = sync_reader(args.port, args.baud)

np.set_printoptions(linewidth=np.inf, threshold=np.inf)

while True:
    print(par.get_msg())
