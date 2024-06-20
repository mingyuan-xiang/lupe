# reference: https://github.com/akamaster/pytorch_resnet_cifar10/tree/master

import torch
import torch.nn as nn
import torch.nn.init as init
import torch.nn.functional as F


def _weights_init(m):
    classname = m.__class__.__name__
    #print(classname)
    if isinstance(m, nn.Linear) or isinstance(m, nn.Conv2d):
        init.kaiming_normal_(m.weight)

class BasicBlock(nn.Module):
    expansion = 1

    def __init__(self, in_planes, planes, stride=1):
        super(BasicBlock, self).__init__()
        self.conv1 = nn.Conv2d(in_planes, planes, kernel_size=3, stride=stride, padding=1, bias=False)
        self.bn1 = nn.BatchNorm2d(planes)
        self.conv2 = nn.Conv2d(planes, planes, kernel_size=3, stride=1, padding=1, bias=False)
        self.bn2 = nn.BatchNorm2d(planes)

        self.stride = stride

        self.shortcut = nn.Sequential()
        if stride != 1 or in_planes != planes:
            self.shortcut = nn.Sequential(
                nn.Conv2d(in_planes, self.expansion * planes, kernel_size=1, stride=stride, bias=False),
                nn.BatchNorm2d(self.expansion * planes)
            )

    def forward(self, x):
        l = []
        x_in = x
        x = self.bn1(self.conv1(x))
        l.append(x.detach().clone())
        x = F.relu(x)
        l.append(x.detach().clone())
        x = self.bn2(self.conv2(x))
        l.append(x.detach().clone())
        x = self.shortcut(x_in)
        if self.stride != 1:
            l.append(x.detach().clone())
        x += x
        l.append(x.detach().clone())
        x = F.relu(x)
        l.append(x.detach().clone())
        # return x, l
        return x, []

class ResNet(nn.Module):
    def __init__(self, block, num_blocks, num_classes=10):
        super(ResNet, self).__init__()
        self.in_planes = 10

        self.conv1 = nn.Conv2d(3, self.in_planes, kernel_size=3, stride=1, padding=1, bias=False)
        self.bn1 = nn.BatchNorm2d(10)
        self.layer1 = self._make_layer(block, 10, num_blocks[0], stride=1)
        self.layer2 = self._make_layer(block, 20, num_blocks[1], stride=2)
        self.layer3 = self._make_layer(block, 40, num_blocks[2], stride=2)
        self.linear = nn.Linear(40, num_classes)

        self.apply(_weights_init)

    def _make_layer(self, block, planes, num_blocks, stride):
        strides = [stride] + [1]*(num_blocks-1)
        layers = []
        for stride in strides:
            layers.append(block(self.in_planes, planes, stride))
            self.in_planes = planes * block.expansion

        return nn.Sequential(*layers)

    def forward(self, x):
        l = [x.detach().clone()]
        x = x.unsqueeze(0)
        x = self.bn1(self.conv1(x))
        l.append(x.detach().clone())
        x = F.relu(x)
        # l.append(x.detach().clone())
        x, ll = self.layer1(x)
        l += ll
        x, ll = self.layer2(x)
        l += ll
        x, ll = self.layer3(x)
        l += ll
        x = F.avg_pool2d(x, int(x.size()[3]))
        # l.append(x.detach().clone())
        x = torch.flatten(x, 1)
        # l.append(x.detach().clone())
        x = self.linear(x)
        # l.append(x.detach().clone())
        return x, l


class ResNet3(ResNet):
    def __init__(self):
        super().__init__(BasicBlock, [1, 1, 1])

        self.names = [
            "input",
            "conv1_Conv",
            # "Clip",
            # "layer1_layer1_0_conv1_Conv",
            # "layer1_layer1_0_Clip",
            # "layer1_layer1_0_conv2_Conv",
            # "layer1_layer1_0_Add",
            # "layer1_layer1_0_Clip_1",
            # "layer2_layer2_0_conv1_Conv",
            # "layer2_layer2_0_Clip",
            # "layer2_layer2_0_conv2_Conv",
            # "layer2_layer2_0_shortcut_shortcut_0_Conv",
            # "layer2_layer2_0_Add",
            # "layer2_layer2_0_Clip_1",
            # "layer3_layer3_0_conv1_Conv",
            # "layer3_layer3_0_Clip",
            # "layer3_layer3_0_conv2_Conv",
            # "layer3_layer3_0_shortcut_shortcut_0_Conv",
            # "layer3_layer3_0_Add",
            # "layer3_layer3_0_Clip_1",
            # "AveragePool",
            # "Flatten",
            # "linear_Gemm",
        ]
