"""send input images for MNIST"""

from torchvision import datasets, transforms

import numpy as np

import sys
sys.path.append("scripts/uart_commute")
from uart_dump import sync_reader, get_args

PRINT_CNT = 100
qf = 2

mnist = datasets.MNIST("./data", train=False,
            transform=transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize((0.5,), (0.5,))
        ]), download=True)

# pack the image to a 1D 16-bit array
def parse_img(img, qf):
    img = img.numpy()
    img = img.flatten() * (2 ** (15 - qf))
    img = img.astype(np.int16)
    return img

args = get_args()
par = sync_reader(args.port, args.baud)

# read start string
while True:
    msg = par.get_msg()
    if msg is not None and "Test" in msg:
        break

print("=============== Start computing ===============")

cnt = 0

# Access each image as a matrix
for idx, (image, label) in enumerate(mnist):
    img_arr = parse_img(image, qf)
    par.put_arr(img_arr)

    l = int(par.get_msg())

    cnt += (l == label)

    print(
        (f"Image {idx}, true label: {label}, predicted label: {l}, " +
        f"accumulative accuracy: {cnt/ (idx + 1)}"))

    if idx % PRINT_CNT == (PRINT_CNT - 1):
        # receive the time
        print(par.get_msg())

# receive end string
print(par.get_msg())

par.close()