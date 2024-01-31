"""Send MNIST test set to msp430"""
from uart_dump import SyncReader, get_args

from torchvision import datasets, transforms
import numpy as np

SCALING = 0.5
PRINT_CNT = 100

# Define a transform to convert the data to tensor format
# transform = transforms.Compose([transforms.ToTensor(),
#                                 transforms.Normalize((0.5,), (0.5,))])
transform = transforms.Compose([transforms.Pad(padding=2),
                                transforms.ToTensor(),
                                transforms.Normalize((0.5,), (0.5,))])

# Load the MNIST training dataset
mnist = datasets.MNIST(root='./data/', train=False, download=True, transform=transform)

def parse_img(img):
    """pack the image to a 1D 16-bit array
    
    Returns:
        Return the 1D numpy array
    """
    img = img.numpy()
    img = img.flatten() * (2 ** 15)
    img = img.astype(np.int16)
    return img

def main():
    """main function"""
    args = get_args()
    par = SyncReader(args.port, args.baud)

    # read start string
    while par.get_msg() != 'Test MNIST':
        pass

    cnt = 0

    # Access each image as a matrix
    for idx, (image, label) in enumerate(mnist):
        img_arr = parse_img(image * SCALING)
        par.put_arr(img_arr)

        l = int(par.get_msg())
        cnt += (l == label)

        print(
            (f'Image {idx}, true label: {label}, predicted label: {l}, '
            f'accumulative accuracy: {cnt/ (idx + 1)}')
        )

        if idx % PRINT_CNT == (PRINT_CNT - 1):
            # receive the time
            print(par.get_msg())
    #        print(par.get_msg())
    #        print(par.get_msg())
    #        print(par.get_msg())
    #        print(par.get_msg())
    #        print(par.get_msg())
    #        print(par.get_msg())
    #        print(par.get_msg())

    # receive end string
    print(par.get_msg())

    par.close()

if __name__ == '__main__':
    main()