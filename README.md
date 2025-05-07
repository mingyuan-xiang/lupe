# Lupe

This is the implementation of our [SenSys' 25 paper](https://dl.acm.org/doi/10.1145/3715014.3722059).
We build our framework based the [CMU maker](https://github.com/CMUAbstract/maker)
with a few customizations. The entire framework is designed for
[TI MSP430FR5994](https://www.ti.com/product/MSP430FR5994?utm_source=google&utm_medium=cpc&utm_campaign=epd-msp-null-44700045336317329_prodfolderdynamic-cpc-pf-google-ww_en_int&utm_content=prodfolddynamic&ds_k=DYNAMIC+SEARCH+ADS&DCM=yes&gclsrc=aw.ds&gad_source=1&gad_campaignid=7213436380&gbraid=0AAAAAC068F1zXtw5KpqBzmPtCc1XNTKmd&gclid=Cj0KCQjw5ubABhDIARIsAHMighYdV1m00bnPcnf-G_YDguCZHepmlfUrIGs9HYlGR-1VxJP3FZ2lm34aAvwvEALw_wcB).

## Environments Setup

We tested and ran all of our experiments on an MacOS as well as an Arch Linux machine.
an TI MSP430FR5994 is necessary to execute Lupe generated programs. Please refer to
[setup](doc/setup.md) for how to set up the dependencies.

## Usage

### Generate Efficient MSP430 Code

You might want to check this [note](doc/notes_for_exp.md) if you want to
replicate the results in our paper.

#### Print the Model

print the model structure with the given name and an ONNX file path.

```
./lupe.py print --model-name <model name> --model-path models/onnx/<model name>.onnx
```

#### Generate DNN Code

The following command generates DNN programs with the given name and an ONNX file path.

+ `--qf` specifies fixed point format.
+ `--config` indicates the location of the optimization configuration file.
  See this [note](doc/config.md) for descriptions of the optimization flags.

```
./lupe.py code-gen --model-name <model name> --model-path models/onnx/<model name>.onnx --qf <qf> --config configs/<config file>
```

There are also some additional arguments that can be used:

+ `--debug-random` will generate a random input for debugging the input shapes
  is specified as `--debug-dataset`.
+ `--intermittent` will enable our lightweight intermittent-safe support.
+ `--hifram-func` will decide how many functions to be put on HIFRAM.
+ `--max-dma-size` sets the maximum DMA size.

One example of the command would be:

```
./lupe.py code-gen --model-name ResNet3 --model-path models/onnx/ResNet3.onnx --qf 4 --config configs/no_opt.json --debug-random --debug-dataset CIFAR10 --intermittent --hifram-func 5
```

### Calibrate DNN Instantiations

As we motivated and explained in Section 2.3 and Section 3.2, we use profiling
to select the best performing DNN instantiations for all layers. It will record
the results in `calibration/<model name>.json`. The optimization flag `adaptive_gen_lea`
needs to be enabled.

The command for calibration is:

```
./lupe.py calibrate --model-name DS_CNN --model-name <model name> --model-path models/onnx/<model name>.onnx --qf <qf> --config configs/<config file>
```

### Run Generated code on the Devices

Change the port of MSP430 in `Makefile.env` to the device that you connected
to your computer. For example, I have:

```
export TI_ROOT = $(PWD)/.toolchains/ti
export FET_PREFIX = usbmodem
export DEVICE_IDX = 1201
```

Because my MSP430 connects to `/dev/cu.usbmodem1201` and `/dev/cu.usbmodem1203`
on my Mac, where "usbmodem1203" is for the UART. You might also want to change
the path of `TI_ROOT` if you change the location of TI toolchains that you
installed from [setup](#environments-setup)

The commands for using maker is:

+ **Compile**: `make apps/<app name>/bld/gcc/all`
+ **Compile and flash**: `make apps/<app name>/bld/gcc/prog`
+ **Clean app**: `make apps/<app name>/bld/gcc/depclean`
+ **Clean app and all dependent libraries**: `make apps/<app name>/bld/gcc/depclean`

## Directory Breakdown

The directory looks like below:

```
.
├── apps
├── calibration
├── codegen
├── configs
├── debug
├── doc
├── ext
├── graph
├── lupe.py
├── Makefile.env
├── makefile.jinja
├── models
├── README.md
├── requirements.txt
├── scripts
└── tools
```

Here are some explanations of the directory:

+ All generated programs are saved in `apps` as it is required by the CMU maker system
+ `calibration` implements most of the logic for calibrating DNN instantiations and
  stores the calibration json files.
+ `codegen` is of the core implementation of Lupe. We use jinja templates as the
  backbone of our framework. The `enhanced_fir.jinja` and `enhanced_mac.jinja`
  implements the batched acceleration idea.
+ `configs` saves the possible optimization flags ([note](doc/config.md)).
+ `debug` provides some helper functions for debugging purposes.
+ `ext` holds C dependencies.
+ `graph` implements the ONNX parsing logic and glue the jinja templates with `lupe.py`
  script.
+ `lupe.py` is our main interface of the framework.
+ `models` provides some ONNX checkpoints that are also used in the paper.
+ `tools` is for the CMU maker

## BibTex

```
@inproceedings{10.1145/3715014.3722059,
    author = {Xiang, Mingyuan and Gholami, Pouya Mahdi and Hoffmann, Henry},
    title = {Lupe: Integrating the Top-down Approach with DNN Execution on Ultra-Low-Power Devices},
    year = {2025},
    isbn = {9798400714795},
    publisher = {Association for Computing Machinery},
    address = {New York, NY, USA},
    url = {https://doi.org/10.1145/3715014.3722059},
    doi = {10.1145/3715014.3722059},
    pages = {357–370},
    numpages = {14},
    keywords = {software optimization, deep neural network (DNN) inference, embedded system, intermittent computing},
    location = {UC Irvine Student Center., Irvine, CA, USA},
    series = {SenSys '25}
}
```


