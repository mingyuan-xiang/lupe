## Set up the environment

### Dependency

+ conda
+ libboost

### Conda environment

```
$ conda env create -f env.yml
$ conda activate lupe
```

### Ti GCC toolchain

```
$ . ./scripts/setup/setup.sh <os>
```

Note:

1. `<os>` has to be `mac` or `linux`
2. The Ti GCC will be put in `.mspgcc` directory
3. The library will be installed in `$HOME/.local`. Make sure to add that directory to your path.
4/ Assume `Homebrew` is installed for mac.

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

TODO