TOOLS_REL_ROOT = tools
# TOOLS =
TOOLCHAINS = gcc

APPS = inter_blink

export DEVICE = msp430fr5994
export BOARD = launchpad 

include tools/maker/Makefile
