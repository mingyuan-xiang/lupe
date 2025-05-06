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

### Generated Efficient MSP430 Code

You might also want to check this [note](doc/notes_for_exp.md) if you want to
replicate the results in our paper.

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


