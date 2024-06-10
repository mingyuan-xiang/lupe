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

# Model

## LeNet

+ Clip(-32, 32) only in training
+ q5.10
+ test accuracy in PyTorch (98.92%)

```
./lupe.py code-gen --model-name LeNet --model-path models/onnx/LeNet.onnx --qf 3
```

# TODO

model clip does not pass to the model constructor (pytorch)
