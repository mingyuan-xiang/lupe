import torch
import torch.nn as nn
import torch.nn.functional as F

class MLPClassifier(nn.Module):
    """
    Reference: https://pytorch.org/tutorials/beginner/introyt/trainingyt.html#the-model
    """
    def __init__(self, clamp_min=-1, clamp_max=1):
        super(MLPClassifier, self).__init__()
        self.conv1 = nn.Conv2d(1, 6, 5)
        self.pool = nn.MaxPool2d(4, 4)
        self.fc1 = nn.Linear(6 * 6 * 6, 200)
        self.fc2 = nn.Linear(200, 120)
        self.fc3 = nn.Linear(120, 84)
        self.fc4 = nn.Linear(84, 10)

        self.clamp_min = clamp_min
        self.clamp_max = clamp_max

    def forward(self, x):
        x = F.relu6(self.conv1(x))
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = self.pool(x)
        x = torch.flatten(x, 1)
        x = F.relu6(self.fc1(x))
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = F.relu(self.fc2(x))
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = F.relu(self.fc3(x))
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = self.fc4(x)
        return x
