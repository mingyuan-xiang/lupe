import torch
import torch.nn as nn
import torch.nn.functional as F

class DS_Block(nn.Module):
    def __init__(self, channels, clamp_min, clamp_max):
        super(DS_Block, self).__init__()
        self.dw_conv = nn.Conv2d(channels, channels, kernel_size=(3, 3), padding=1, groups=channels)
        self.dw_bn = nn.BatchNorm2d(channels)
        self.dw_relu = nn.ReLU6(inplace=True)
        self.pw_conv = nn.Conv2d(channels, channels, kernel_size=(1, 1))
        self.pw_bn = nn.BatchNorm2d(channels)
        self.pw_relu = nn.ReLU6(inplace=True)

        self.clamp_min = clamp_min
        self.clamp_max = clamp_max

    def forward(self, x):
        out = self.dw_bn(self.dw_conv(x))
        out = torch.clamp(out, min=self.clamp_min, max=self.clamp_max)
        out = self.dw_relu(out)
        out = self.pw_bn(self.pw_conv(out))
        out = torch.clamp(out, min=self.clamp_min, max=self.clamp_max)
        out = self.pw_relu(out)
        return out

class DS_CNN(nn.Module):
    def __init__(self, clamp_min, clamp_max):
        super(DS_CNN, self).__init__()
        channels = 64
        
        self.conv1 = nn.Conv2d(1, channels, kernel_size=(10, 4), stride=(2, 2), padding=1)
        self.bn1 = nn.BatchNorm2d(channels)
        self.relu1 = nn.ReLU6(inplace=True)
        
        self.block1 = DS_Block(channels, clamp_min, clamp_max)
        self.block2 = DS_Block(channels, clamp_min, clamp_max)
        self.block3 = DS_Block(channels, clamp_min, clamp_max)
        self.block4 = DS_Block(channels, clamp_min, clamp_max)
        self.avgpool = nn.AdaptiveAvgPool2d((1, 1))
        self.classifier = nn.Linear(channels, 11)

        self.clamp_min = clamp_min
        self.clamp_max = clamp_max

    def forward(self, x):
        out = self.bn1(self.conv1(x))
        out = torch.clamp(out, min=self.clamp_min, max=self.clamp_max)
        out = self.relu1(out)
        if self.train:
            out = F.dropout(out, p=0.2, inplace=True)
        out = self.block1(out)
        out = self.block2(out)
        out = self.block3(out)
        out = self.block4(out)
        if self.train:
            out = F.dropout(out, p=0.4, inplace=True)
        out = self.avgpool(out)
        out = torch.flatten(out, 1)
        out = self.classifier(out)
        return out
