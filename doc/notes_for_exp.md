- [Notes for Replicating Our Results](#notes-for-replicating-our-results)
  - [Caveat](#caveat)
  - [Converting PyTorch Model to ONNX](#converting-pytorch-model-to-onnx)
  - [Some parameters we use when generating DNN programs](#some-parameters-we-use-when-generating-dnn-programs)
    - [LeNet](#lenet)
    - [ResNet3](#resnet3)
    - [MobileNetV2](#mobilenetv2)
    - [DS-CNN](#ds-cnn)
    - [MLPClassifier](#mlpclassifier)
  - [Intermittent Time](#intermittent-time)

# Notes for Replicating Our Results

## Caveat

In general, the ONNX model I provided in [models/onnx](../models/onnx) should
all work. Those files are getting from model architectures that I use in
[models/models](../models/models).

However, although we try to make our framework as general
as possible, there is too much engineering for me to make a extremely general
framework that works for every ONNX model. So, my framework doesn't support
implementations of the same DNN layers because there is simply too many of them,
and working on parsing ONNX models won't lead to anything interesting.

Still, there are some very common layers are not properly supported.
For example, we could flatten a DNN layer by either `torch.flatten(x, 1)` or
`x.view(x.size(0), -1)`. These two functions are encoded in two different
ONNX formats, but my parser only recognizes `torch.flatten(x, 1)`.

Please use model architectures in [models/models](../models/models) as a reference.
Those should all work.
Nevertheless, open a GitHub issue if you realize the ONNX layer are not supported.
I'm happy to help you out.

## Converting PyTorch Model to ONNX

The script `to_onnx.py` do everything for you. For example:

```
python to_onnx.py --dataset cifar10 -a <model name> --param-loc checkpoints/<model name>/model_best.pth
```

This will convert a PyTorch mode

## Some parameters we use when generating DNN programs

We benchmark on 5 different models and we use different [fixed point format](https://en.wikipedia.org/wiki/Q_(number_format))
(Qm.n) for different models to get the best accuracy because everything needs
to be computed in integers on the MSP430. We also clip the output values for
each DNN layer during training, though the clipping range varies.

### LeNet

+ mnist size: 10000
+ Clip(-32, 32) only in training
+ q5.10 for activation and bias, `qf=5`, `qf_offset=1` for weights
+ test accuracy in PyTorch (98.49%)

### ResNet3

+ cifar10 size: 10000
+ Clip(-16, 16) for activation and Clip(-1, 1) for weights (only in training)
+ q4.11 for activation and bias, `qf=4`, `qf_offset=1` for weights
+ test accuracy in PyTorch (80.42%)

### MobileNetV2

+ vww size: 8059
+ Clip(-16, 16) for activation and Clip(-1, 1) for weights (only in training)
+ q4.11 for activation and bias, `qf=4`, `qf_offset=1` for weights
+ test accuracy in PyTorch (80.69%)
+ (bottom-up implementation) For intermittent-safe support, it cannot fit, so I have to put 10 layer functions on the HIFRAM (`--hifram-func 10`)

### DS-CNN

+ speech_commands size: 11005
+ Clip(-32, 32) for activation and Clip(-1, 1) for weights (only in training)
+ q5.10 for activation and bias, `qf=5`, `qf_offset=1` for weights
+ test accuracy in PyTorch (94.39%)

### MLPClassifier

+ fasion_mnist: 10000
+ Clip(-32, 32) only in training
+ q5.10 for activation and bias, `qf=5`, `qf_offset=1` for weights
+ test accuracy in PyTorch (90.57%)

## Intermittent Time

+ For restart frequency in 5 - 10 ms, the DMA for no optimization case is too
  slow, so we need to set `max-dma-size` to be 2000.