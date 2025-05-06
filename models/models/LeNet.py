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
    def __init__(self, clamp_min=-1, clamp_max=1):
        super(LeNet, self).__init__()
        self.conv1 = nn.Conv2d(1, 6, 5, padding=2)
        self.relu1 = nn.ReLU6()
        self.pool1 = nn.AvgPool2d(2, 2)
        self.conv2 = nn.Conv2d(6, 16, 5)
        self.relu2 = nn.ReLU6()
        self.pool2 = nn.AvgPool2d(2, 2)
        self.conv3 = nn.Conv2d(16, 120, 5)
        self.relu3 = nn.ReLU6()
        self.fc1 = nn.Linear(120, 84)
        self.relu4 = nn.ReLU6()
        self.fc2 = nn.Linear(84, 10)

        self.clamp_min = clamp_min
        self.clamp_max = clamp_max


    def forward(self, x):
        x = self.relu1(self.conv1(x))
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = self.pool1(x)
        x = self.relu2(self.conv2(x))
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = self.pool2(x)
        x = self.relu3(self.conv3(x))
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = torch.flatten(x, 1)
        x = self.relu4(self.fc1(x))
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = self.fc2(x)
        return x
