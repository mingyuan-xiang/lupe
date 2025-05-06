# Environments Setup

The dependency setup consists of two parts: 1) Python environments to generate
MSP430 code; 2) CMU maker to run generated code on an MSP430.

## Python Environments

This part is as simple as create a Python virtual environments and install
all requirements in `requirements.txt`.

```
$ python3 -m venv .env
$ source .env/bin/activate
$ pip install -r requirements.txt
```

## Setup CMU maker

It is a bit tricky to use CMU maker, but I think this [script](scripts/setup/setup.sh)
would solve most of the dependency issue. The whole setup requires [TI's CCS](https://www.ti.com/tool/CCSTUDIO?utm_source=google&utm_medium=cpc&utm_campaign=epd-der-null-44700045336317965_prodfolderdynamic-cpc-pf-google-ww_en_int&utm_content=prodfolddynamic&ds_k=DYNAMIC+SEARCH+ADS&DCM=yes&gclsrc=aw.ds&gad_source=1&gad_campaignid=12788797621&gbraid=0AAAAAC068F2Oc5buYmPOiiXPkiyt64KOQ&gclid=Cj0KCQjw5ubABhDIARIsAHMigha7qvt0TmqkCyi_52v4Dh8IkMQLFBhDepp4rMnpgIaMNq-2glBnPhgaAolAEALw_wcB)
is installed and opened for at least once to install some random TI libraries
for the MSP430 driver.

===============================================================================

### !!Caveat!!

I only one MacOS and it is a bit hard for me to have a fresh environment
for either MacOS or Arch Linux, so the you might need to solve some dependency
issues on your own. However, I'm happy to help you out. Just shoot me an email
or open an GitHub issue.

Some of the known dependencies that needs to be installed in advance for my
[setup script](scripts/setup/setup.sh) to work are `pkg-config` and `cmake`, but
there might be more.

Alternatively, you could copy and paste the generated programs into CCS and
start from there if you don't like CMU maker.

===============================================================================

Assume the python environment is created. Then, simply do the following in the
root directory of Lupe directory. 

MacOS:

```
$ . ./scripts/setup/setup.sh mac
```

Arch Linux:

```
$ . ./scripts/setup/setup.sh linux
```

Hopefully, it should automatically install everything in the virtual environments
you just created (including the GCC toolchains for MSP430). The environment
should be automatically exported when you enable the Python virtual environments,
namely when you do `source .env/bin/activate`.

