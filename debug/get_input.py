"""Get the first input image(data) of a given dataset"""

from torchvision import datasets
from torchvision import transforms

def get_input(dataset, idx):
    """Return the {idx} input image(data) in a numpy array"""
    if dataset.lower() == "mnist":
        d = datasets.MNIST('./data', train=False,
            transform=transforms.Compose([
            transforms.ToTensor(),
            transforms.Normalize((0.5,), (0.5,))
        ]), download=True)
    else:
        raise NotImplementedError(f"{dataset} is not supported")

    arr, label = d[idx]

    return arr.cpu().detach().numpy(), label
