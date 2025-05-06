"""Get the first input image(data) of a given dataset"""
import sys

import torch
from torchvision import datasets
from torchvision import transforms
import pyvww

sys.path.append("models/dataset")
from speech_commands import SpeechCommands

def get_input(dataset, idx, random=False, seed=0):
    """Return the {idx} input image(data) in a numpy array"""
    if random:
        torch.manual_seed(seed)

        if dataset.lower() == 'mnist':
            dummy_input = torch.randn(1, 1, 28, 28)
        elif dataset.lower() == 'cifar10':
            dummy_input = torch.randn(1, 3, 32, 32)
        elif dataset.lower() == 'vww':
            dummy_input = torch.randn(1, 3, 80, 80)
        elif dataset.lower() == 'sc':
            dummy_input = torch.randn(1, 1, 49, 12)
        elif dataset.lower() == 'fashion_mnist':
            dummy_input = torch.randn(1, 1, 28, 28)
        else:
            raise ValueError('Invalid dataset')

        return dummy_input.cpu().detach().numpy(), 0
    else:
        if dataset.lower() == "mnist":
            d = datasets.MNIST('./data', train=False,
                transform=transforms.Compose([
                transforms.ToTensor(),
                transforms.Normalize((0.5,), (0.5,))
            ]), download=True)
        elif dataset.lower() == "cifar10":
            d = datasets.CIFAR10(
                root='./data', train=False, transform=transforms.Compose([
                transforms.ToTensor(),
                transforms.Normalize(mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225])
            ]), download=True)
        elif dataset.lower() == "vww":
            normalize = transforms.Normalize(mean=[0.485, 0.456, 0.406],
                std=[0.229, 0.224, 0.225])
            d = pyvww.pytorch.VisualWakeWordsClassification(
                root='models/data/vww/all2017/',
                annFile='models/data/vww/annotations/instances_val.json',
                transform=transforms.Compose([
                    transforms.Resize((80, 80)),
                    transforms.ToTensor(),
                    normalize,
                ]))
        elif dataset.lower() == "sc":
            d = SpeechCommands('./models/data/SpeechCommands', 'testing')
        elif dataset.lower() == "fashion_mnist":
            transform = transforms.Compose(
                [transforms.ToTensor(),
                transforms.Normalize((0.5,), (0.5,))])

            d = datasets.FashionMNIST(
                './data', train=False, transform=transform, download=True)
        else:
            raise NotImplementedError(f"{dataset} is not supported")

        arr, label = d[idx]

        return arr.cpu().detach().numpy(), label
