from .mobilenet import MobileNet

class MobileNetV2(MobileNet):
    def __init__(self, clamp_min=-1, clamp_max=1):
        super().__init__(num_classes=2, width_mult=0.25, clamp_min=clamp_min, clamp_max=clamp_max)
