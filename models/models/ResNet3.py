from .resnet import ResNet, BasicBlock

class ResNet3(ResNet):
    def __init__(self, clamp_min=-1, clamp_max=1):
        super().__init__(BasicBlock, [1, 1, 1], clamp_min=clamp_min, clamp_max=clamp_max)
