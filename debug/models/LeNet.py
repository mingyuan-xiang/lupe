import torch
import torch.nn as nn

class LeNet(nn.Module):
    """
    The network is listed below:
        + Conv 6 * 1 * 5 * 5
        + AvgPool 6 * 14 * 14
        + Conv 16 * 6 * 5 * 5
        + AvgPool 16 * 5 * 5
        + Conv 120 * 16 * 5 * 5
        + FC 84 * 120
        + FC 10 * 84
    """
    def __init__(self):
        super(LeNet, self).__init__()
        self.conv1 = nn.Conv2d(1, 6, 5, padding=2)
        self.pool1 = nn.AvgPool2d(2, 2)
        self.conv2 = nn.Conv2d(6, 16, 5)
        self.pool2 = nn.AvgPool2d(2, 2)
        self.conv3 = nn.Conv2d(16, 120, 5)
        self.fc1 = nn.Linear(120, 84)
        self.fc2 = nn.Linear(84, 10)
        self.names = [
            "input",
            "conv1",
            "relu1",
            "pool1",
            "conv2",
            "relu2",
            "pool2",
            "conv3",
            "relu3",
            "flatten",
            "fc1",
            "relu3",
            "fc2",
        ]

    def forward(self, x):
        l = [x.detach().clone()]
        x = self.conv1(x)
        l.append(x.detach().clone())
        x = torch.relu(x)
        l.append(x.detach().clone())
        x = self.pool1(x)
        l.append(x.detach().clone())
        x = self.conv2(x)
        l.append(x.detach().clone())
        x = torch.relu(x)
        l.append(x.detach().clone())
        x = self.pool2(x)
        l.append(x.detach().clone())
        x = self.conv3(x)
        l.append(x.detach().clone())
        x = torch.relu(x)
        l.append(x.detach().clone())
        x = torch.flatten(x, 0)
        l.append(x.detach().clone())
        x = self.fc1(x)
        l.append(x.detach().clone())
        x = torch.relu(x)
        l.append(x.detach().clone())
        x = self.fc2(x)
        l.append(x.detach().clone())
        return x, l
