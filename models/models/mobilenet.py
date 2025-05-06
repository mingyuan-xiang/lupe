# Reference: https://github.com/d-li14/mobilenetv2.pytorch/blob/master/models/imagenet/mobilenetv2.py

import torch
import torch.nn as nn
import math

def _make_divisible(v, divisor, min_value=None):
    """
    This function is taken from the original tf repo.
    It ensures that all layers have a channel number that is divisible by 8
    It can be seen here:
    https://github.com/tensorflow/models/blob/master/research/slim/nets/mobilenet/mobilenet.py
    :param v:
    :param divisor:
    :param min_value:
    :return:
    """
    if min_value is None:
        min_value = divisor
    new_v = max(min_value, int(v + divisor / 2) // divisor * divisor)
    # Make sure that round down does not go down by more than 10%.
    if new_v < 0.9 * v:
        new_v += divisor
    return new_v


class Conv2D_3x3_BN(nn.Module):
    def __init__(self, inp, oup, stride, clamp_min, clamp_max):
        super(Conv2D_3x3_BN, self).__init__()
        self.block = nn.Sequential(
            nn.Conv2d(inp, oup, 3, stride, 1, bias=False),
            nn.BatchNorm2d(oup),
            nn.ReLU6(inplace=True)
        )

        self.clamp_min = clamp_min
        self.clamp_max = clamp_max

    def forward(self, x):
        out = self.block(x)
        out = torch.clamp(out, min=self.clamp_min, max=self.clamp_max)
        return out

class Conv2D_1x1_BN(nn.Module):
    def __init__(self, inp, oup, clamp_min, clamp_max):
        super(Conv2D_1x1_BN, self).__init__()
        self.block = nn.Sequential(
            nn.Conv2d(inp, oup, 1, 1, 0, bias=False),
            nn.BatchNorm2d(oup),
            nn.ReLU6(inplace=True)
        )

        self.clamp_min = clamp_min
        self.clamp_max = clamp_max

    def forward(self, x):
        out = self.block(x)
        out = torch.clamp(out, min=self.clamp_min, max=self.clamp_max)
        return out

class InvertedResidual(nn.Module):
    def __init__(self, inp, oup, stride, expand_ratio, clamp_min, clamp_max):
        super(InvertedResidual, self).__init__()
        assert stride in [1, 2]

        self.clamp_min = clamp_min
        self.clamp_max = clamp_max

        self.expand_ratio = expand_ratio
        hidden_dim = round(inp * expand_ratio)
        self.identity = stride == 1 and inp == oup

        if expand_ratio == 1:
            self.dw_conv = nn.Sequential(
                nn.Conv2d(hidden_dim, hidden_dim, 3, stride, 1, groups=hidden_dim, bias=False),
                nn.BatchNorm2d(hidden_dim),
                nn.ReLU6(inplace=True),
            )
            self.pw_conv_linear = nn.Sequential(
                nn.Conv2d(hidden_dim, oup, 1, 1, 0, bias=False),
                nn.BatchNorm2d(oup),
            )
        else:
            self.pw_conv = nn.Sequential(
                nn.Conv2d(inp, hidden_dim, 1, 1, 0, bias=False),
                nn.BatchNorm2d(hidden_dim),
                nn.ReLU6(inplace=True),
            )
            self.dw_conv = nn.Sequential(
                nn.Conv2d(hidden_dim, hidden_dim, 3, stride, 1, groups=hidden_dim, bias=False),
                nn.BatchNorm2d(hidden_dim),
                nn.ReLU6(inplace=True),
            )
            self.pw_conv_linear = nn.Sequential(
                nn.Conv2d(hidden_dim, oup, 1, 1, 0, bias=False),
                nn.BatchNorm2d(oup),
            )

    def forward(self, x):
        if self.expand_ratio != 1:
            out = self.pw_conv(x)
            out = torch.clamp(out, min=self.clamp_min, max=self.clamp_max)
        else:
            out = x

        out = self.dw_conv(out)
        out = torch.clamp(out, min=self.clamp_min, max=self.clamp_max)

        out = self.pw_conv_linear(out)
        out = torch.clamp(out, min=self.clamp_min, max=self.clamp_max)

        if self.identity:
            return x + out
        else:
            return out


class MobileNet(nn.Module):
    def __init__(self, num_classes=10, width_mult=1., clamp_min=-1, clamp_max=1):
        super(MobileNet, self).__init__()
        self.clamp_min = clamp_min
        self.clamp_max = clamp_max

        # setting of inverted residual blocks
        self.cfgs = [
            # t, c, n, s
            [1,  16, 1, 2],
            [6,  24, 1, 2],
            [6,  32, 1, 2],
            [6,  64, 2, 2],
            [6,  96, 1, 1],
        ]

        # building first layer
        input_channel = _make_divisible(32 * width_mult, 4 if width_mult == 0.1 else 8)
        layers = [Conv2D_3x3_BN(3, input_channel, 2, self.clamp_min, self.clamp_max)]
        # building inverted residual blocks
        block = InvertedResidual
        for t, c, n, s in self.cfgs:
            output_channel = _make_divisible(c * width_mult, 4 if width_mult == 0.1 else 8)
            for i in range(n):
                layers.append(block(input_channel, output_channel, s if i == 0 else 1, t, self.clamp_min, self.clamp_max))
                input_channel = output_channel
        self.features = nn.Sequential(*layers)
        # building last several layers
        last_channels = 160
        output_channel = _make_divisible(last_channels * width_mult, 4 if width_mult == 0.1 else 8) if width_mult > 1.0 else last_channels
        self.conv = Conv2D_1x1_BN(input_channel, output_channel, self.clamp_min, self.clamp_max)
        self.avgpool = nn.AdaptiveAvgPool2d((1, 1))
        self.classifier = nn.Linear(output_channel, num_classes)

        self._initialize_weights()

    def forward(self, x):
        x = self.features(x)
        x = self.conv(x)
        x = torch.clamp(x, min=self.clamp_min, max=self.clamp_max)
        x = self.avgpool(x)
        x = torch.flatten(x, 1)
        x = self.classifier(x)
        return x

    def _initialize_weights(self):
        for m in self.modules():
            if isinstance(m, nn.Conv2d):
                n = m.kernel_size[0] * m.kernel_size[1] * m.out_channels
                m.weight.data.normal_(0, math.sqrt(2. / n))
                if m.bias is not None:
                    m.bias.data.zero_()
            elif isinstance(m, nn.BatchNorm2d):
                m.weight.data.fill_(1)
                m.bias.data.zero_()
            elif isinstance(m, nn.Linear):
                m.weight.data.normal_(0, 0.01)
                m.bias.data.zero_()
