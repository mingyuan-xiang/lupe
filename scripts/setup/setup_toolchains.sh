#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: setup_toolchains.sh <linux | mac> <bin path>"
  return 1
fi
if [ "$1" != "linux" ] && [ "$1" != "mac" ]; then
  echo "Invalid OS. Please use 'linux' or 'mac'"
  return 1
fi

echo "installing mspgcc"

DIR=tmp_download
mkdir -p ".toolchains/ti/mspgcc"
TOOLCHAIN_DIR=`readlink -f ".toolchains/ti/mspgcc"`

mkdir -p $DIR
pushd $DIR >/dev/null

if [[ "$1" == "linux" ]]; then
  GCC_URL=https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-LlCjWuAbzH/9.3.1.2/msp430-gcc-9.3.1.11_linux64.tar.bz2
  GCC_DIRNAME=msp430-gcc-9.3.1.11_linux64
else
  GCC_URL=https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-LlCjWuAbzH/9.3.1.2/msp430-gcc-9.3.1.11_macos.tar.bz2
  GCC_DIRNAME=msp430-gcc-9.3.1.11_macos
fi
GCC_SUP_URL=https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-LlCjWuAbzH/9.3.1.2/msp430-gcc-support-files-1.212.zip

wget --quiet $GCC_URL
wget --quiet $GCC_SUP_URL

tar xjf "$GCC_DIRNAME.tar.bz2"
unzip -qx msp430-gcc-support-files-1.212.zip
mv msp430-gcc-support-files/include/* $GCC_DIRNAME/include

mkdir -p $TOOLCHAIN_DIR
mv $GCC_DIRNAME/* $TOOLCHAIN_DIR

mkdir -p $2
pushd $TOOLCHAIN_DIR >/dev/null
stow -t $2 bin

popd >/dev/null
popd >/dev/null

rm -rf tmp_download