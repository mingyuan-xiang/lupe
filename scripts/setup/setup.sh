#!/bin/bash

verify_input() {
  if [ "$#" -ne 1 ]; then
    echo "Usage: set_up_gcc.sh <linux | mac>"
    exit 1
  fi
  if [ "$1" != "linux" ] && [ "$1" != "mac" ]; then
    echo "Invalid OS. Please use 'linux' or 'mac'"
    exit 1
  fi
}

install_deps() {
  if [[ "$1" == "linux" ]]; then
    echo "Not implemented yet."
  else
    local BREW_INSTALL=""
    for d in ${@:2}; do
      echo $d
      if brew ls $d > /dev/null; then
        echo "${d} already installed"
      else
        BREW_INSTALL+=$d
      fi
    done;
    if [ "$BREW_INSTALL" != "" ]; then
      brew install $BREW_INSTALL
    fi 
  fi
}

main() {
  verify_input $@

  mkdir -p ".mspgcc/"
  TOOLCHAIN_DIR=`readlink -f ".mspgcc/"`

  DIR="tmp_downloads"
  mkdir -p $DIR
  pushd $DIR >/dev/null

  ## Install mspdebug
  if ! hash mspdebug_unlinked 2>/dev/null; then
    echo "installing mspdebug deps"
    install_deps $1 "hidapi" "libusb-compat"

    echo "installing mspdebug"
    git clone https://github.com/dlbeer/mspdebug
    cd mspdebug

    # hidapi fix for mspdebug
    export CFLAGS="-I/opt/homebrew/include"

    # make
    make --silent PREFIX=$CONDA_PREFIX install
    mv $CONDA_PREFIX/bin/mspdebug $CONDA_PREFIX/bin/mspdebug_unlinked

    cd ..
    echo "mspdebug_unlinked installed"
  fi

  ## Install the required dylib
  if [ ! -f $CONDA_PREFIX/lib/libmsp430.dylib ]; then
    echo "installing libmsp430.dylib deps"
    install_deps $1 "hidapi" "boost"

    echo "installing libmsp430.dylib"
    wget --quiet "https://dr-download.ti.com/software-development/driver-or-library/MD-4vnqcP1Wk4/3.15.1.1/MSPDebugStack_OS_Package_3_15_1_1.zip"
    unzip -q MSPDebugStack_OS_Package_3_15_1_1.zip -d mspds
    cd mspds

    HID_PATH=`brew --prefix hidapi`
    HID_PATH=$HID_PATH/include/hidapi/hidapi.h
    cp $HID_PATH ./ThirdParty/include/

    cp ../../scripts/setup/mspds_makefile ./Makefile

    make --silent BOOST_DIR=/opt/homebrew 
    make --silent PREFIX=$CONDA_PREFIX install

    cd ..
    echo "libmsp430.dyblib installed"
  fi

  # Install the msp430 libraries
  if ! hash msp430-elf-gcc 2>/dev/null; then
    echo "installing mspgcc"
    wget --quiet https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-LlCjWuAbzH/9.3.1.2/msp430-gcc-9.3.1.11_macos.tar.bz2
    wget --quiet https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-LlCjWuAbzH/9.3.1.2/msp430-gcc-support-files-1.212.zip

    tar xjf msp430-gcc-9.3.1.11_macos.tar.bz2
    unzip -qx msp430-gcc-support-files-1.212.zip
    mv msp430-gcc-support-files/include/* msp430-gcc-9.3.1.11_macos/include

    mkdir -p $TOOLCHAIN_DIR
    mv msp430-gcc-9.3.1.11_macos/* $TOOLCHAIN_DIR

    cd $TOOLCHAIN_DIR
    stow -t $CONDA_PREFIX/bin bin
  fi

  if ! hash mspdebug 2>/dev/null; then
    echo "installing linked mspdebug"
    mspdebug=$CONDA_PREFIX/bin/mspdebug
    touch $mspdebug
    echo "#!/bin/bash" > $mspdebug
    echo "DYLD_LIBRARY_PATH=$CONDA_PREFIX/lib mspdebug_unlinked \"\$@\"" >> $mspdebug
    chmod +x $mspdebug
  fi

  popd >/dev/null
  rm -rf $DIR
}

main $@