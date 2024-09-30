# Set up the environment

## Dependency

+ conda is required to manage the python environment

## Conda environment

```
$ conda env create -f < mac_env.yml | archlinux_env.yml >
$ conda activate lupe
```

## Set up CMU [maker](https://github.com/CMUAbstract/maker) to compile and flash

Note:

1. The Ti GCC will be put in `.toolchains/ti/mspgcc/` directory

### For MacOS

Run the following command will install dependencies for the maker. (Assuming [**Homebrew**](https://brew.sh/) is installed)

```
$ . ./scripts/setup/setup.sh mac
```

### For Linux

#### Arch Linux

**Step 1**

Ti CCS needs to be installed to initialize the driver (with root access).
Check: [https://aur.archlinux.org/packages/ccstudio](https://aur.archlinux.org/packages/ccstudio)

**Step 2**

Install the following library:

+ [https://aur.archlinux.org/packages/mspdebug](https://aur.archlinux.org/packages/mspdebug)
+ [https://aur.archlinux.org/packages/mspds](https://aur.archlinux.org/packages/mspds)

**Step 3**

Install the mspgcc

`<bin directory>` is where the mspgcc will be put

```
$ . ./scripts/setup/setup_toolchains.sh linux <bin directory>
```


#### Ubuntu

**Step 1**

Ti CCS needs to be installed to initialize the driver (with root access).
Check: [https://software-dl.ti.com/ccs/esd/documents/ccsv11_linux_host_support.html#after-installation](https://software-dl.ti.com/ccs/esd/documents/ccsv11_linux_host_support.html#after-installation)

**Step 2**

Run the following command will install decencies for the maker (without root access).
The library will be installed in `$HOME/.local` for ubuntu. Make sure to add it to your path.

```
$ . ./scripts/setup/setup.sh linux
```

**Note**: Haven't tested the ubuntu yet, though it should work.

## MSP430 port

Change the port of MSP430 in `Makefile.env` correspondingly.

# compile

```
make apps/<app name>/bld/gcc/all
```

# compile and flash

```
make apps/<app name>/bld/gcc/prog
```

# clean

```
make apps/<app name>/bld/gcc/depclean
```

# Python requirements

# Print the model

```
./lupe.py print --model-name LeNet --model-path models/onnx/LeNet.onnx
```

# Run experiments

```
python scripts/run_experiments.py --model <model name> --dataset <dataset> --qf <qf>
```

# Model

## LeNet

+ mnist size: 10000
+ Clip(-32, 32) only in training
+ q5.10 for activation and bias, `qf_offset=1` for weights
+ test accuracy in PyTorch (98.49%)

## ResNet3

+ cifar10 size: 10000
+ Clip(-16, 16) for activation and Clip(-1, 1) for weights (only in training)
+ q4.11 for activation and bias, `qf_offset=1` for weights
+ test accuracy in PyTorch (80.42%)

## MobileNetV2

+ vww size: 8059
+ Clip(-16, 16) for activation and Clip(-1, 1) for weights (only in training)
+ q4.11 for activation and bias, `qf_offset=1` for weights
+ test accuracy in PyTorch (80.69%)
+ (bottom-up implementation) For intermittent-safe support, it cannot fit, so I have to put 10 layer functions on the HIFRAM (`--hifram-func 10`)

## DS-CNN

+ speech_commands size: 11005
+ Clip(-32, 32) for activation and Clip(-1, 1) for weights (only in training)
+ q5.10 for activation and bias, `qf_offset=1` for weights
+ test accuracy in PyTorch (94.39%)

```
./lupe.py code-gen --model-name LeNet --model-path models/onnx/LeNet.onnx --qf 3
```

## MLPClassifier

+ fasion_mnist: 10000
+ Clip(-32, 32) only in training
+ q5.10 for activation and bias, `qf_offset=1` for weights
+ test accuracy in PyTorch (90.57%)

# TODO

model clip does not pass to the model constructor (pytorch)
