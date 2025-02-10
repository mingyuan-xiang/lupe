#!/bin/bash

#TODO: install `pkg-config` on mac
#TODO: There is a bug with boost 1.87.0 library (https://github.com/PointCloudLibrary/pcl/issues/6202),
# which is the version when I install it with homebrew.
# So, I use version 1.85.0 instead (MSP_BOOST_DIR="/opt/homebrew/opt/boost@1.85/").

verify_input() {
  if [ "$#" -ne 1 ]; then
    echo "Usage: setup.sh <linux | mac>"
    return 1
  fi
  if [ "$1" != "linux" ] && [ "$1" != "mac" ]; then
    echo "Invalid OS. Please use 'linux' or 'mac'"
    return 1
  fi
}

export LINUX_PREFIX=$VIRTUAL_ENV

linux_install() {
  if [[ "$1" == "hidapi" ]]; then
    if [ -d "$LINUX_PREFIX/include/hidapi" ] ; then
      echo "hidapi already installed!"
      return 0
    fi

    export PKG_CONFIG_PATH=$LINUX_PREFIX/lib/pkgconfig/
    git clone https://github.com/libusb/hidapi.git
    cd hidapi
    ./bootstrap
    ./configure --prefix=$LINUX_PREFIX
    make
    make install
    cd ..
  elif [[ "$1" == "libusb-compat" ]]; then
    if [ -f "$LINUX_PREFIX/include/usb.h" ] ; then
      echo "libusb-compat already installed!"
      return 0
    fi

    git clone https://github.com/libusb/libusb.git
    cd libusb
    ./autogen.sh
    ./configure --prefix=$LINUX_PREFIX
    make
    make install
    cd ..

    export PKG_CONFIG_PATH=$LINUX_PREFIX/lib/pkgconfig/
    git clone https://github.com/libusb/libusb-compat-0.1.git
    cd libusb-compat-0.1
    ./autogen.sh
    ./configure --prefix=$LINUX_PREFIX
    make
    make install
    cd ..
  elif [[ "$1" == "boost" ]]; then
    if dpkg -s libboost-dev | grep -q "Version" ;  then
      echo "libboost already installed!"
      export MSP_BOOST_DIR=/usr/lib/x86_64-linux-gnu/
      export MSP_BOOST_INCLUDE=/usr/include/boost/
      return 0
    else
      echo "Please install libboost"
      return 1
    fi
  elif [[ "$1" == "stow" ]]; then
    if [ -d "$LINUX_PREFIX/include/stow" ] ; then
      echo "stow already installed!"
      return 0
    fi

    git clone https://github.com/aspiers/stow.git
    cd stow
    autoreconf -iv
    ./configure --prefix=$LINUX_PREFIX
    make
    make install
    cd ..
  else
    echo "Installing $1 not implemented!"
    return 1
  fi
}

install_deps() {
  if [[ "$1" == "linux" ]]; then
    export LD_LIBRARY_PATH="$LINUX_PREFIX/lib/:$LD_LIBRARY_PATH"
    export LD_RUN_PATH="$LINUX_PREFIX/lib/:$LD_RUN_PATH"

    for d in ${@:2}; do
      linux_install $d
      if [ $? -eq 1 ]; then
        return 1
      fi
    done;
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
  if [ $? -eq 1 ]; then
    return 1
  fi

  ## Install mspdebug
  if ! hash mspdebug_unlinked 2>/dev/null; then
    echo "installing mspdebug deps"
    install_deps $1 "libusb-compat" "hidapi" 
    if [ $? -eq 1 ]; then
      return 1
    fi

    echo "installing mspdebug"
    git clone https://github.com/dlbeer/mspdebug
    cd mspdebug

    # hidapi fix for mspdebug
    if [[ "$1" == "linux" ]]; then
      export PKG_CONFIG_PATH=$LINUX_PREFIX/lib/pkgconfig
      export CFLAGS=$(pkg-config --cflags libusb hidapi-libusb)
      LIB_FLAGS=$(pkg-config --libs libusb hidapi-libusb)
      sed -i "s\-lusb\ $LIB_FLAGS\g" Makefile
    else
      export CFLAGS="-I/opt/homebrew/include"
    fi

    # make
    make --silent PREFIX=$VIRTUAL_ENV install
    mv $VIRTUAL_ENV/bin/mspdebug $VIRTUAL_ENV/bin/mspdebug_unlinked

    cd ..
    echo "mspdebug_unlinked installed"
  fi

  if [[ "$1" == "linux" ]]; then
    MSP_LIB=libmsp430.so
    HID_PATH=$LINUX_PREFIX/include/hidapi/hidapi.h
    MSP_BOOST_DIR=/usr/lib/x86_64-linux-gnu/
  else
    MSP_LIB=libmsp430.dylib
    HID_PATH=`brew --prefix hidapi`
    HID_PATH=$HID_PATH/include/hidapi/hidapi.h
    MSP_BOOST_DIR="/opt/homebrew/opt/boost@1.85/"
  fi

  ## Install the required dylib
  if [ ! -f $VIRTUAL_ENV/lib/$MSP_LIB ]; then
    echo "installing $MSP_LIB deps"
    install_deps $1 "hidapi" "boost"
    if [ $? -eq 1 ]; then
      return 1
    fi

    echo "installing $MSP_LIB"
    wget --quiet "https://dr-download.ti.com/software-development/driver-or-library/MD-4vnqcP1Wk4/3.15.1.1/MSPDebugStack_OS_Package_3_15_1_1.zip"
    unzip -q "MSPDebugStack_OS_Package_3_15_1_1.zip" -d mspds
    cd mspds

    cp $HID_PATH ./ThirdParty/include/
    
    cp ../../scripts/setup/mspds_makefile ./Makefile

    if [[ "$1" == "linux" ]]; then
      export PKG_CONFIG_PATH=$LINUX_PREFIX/lib/pkgconfig/
      make --silent BOOST_DIR=$MSP_BOOST_DIR BOOST_INCLUDE=$MSP_BOOST_INCLUDE
    else
      export PKG_CONFIG_PATH=/opt/homebrew/lib/pkgconfig/
      make --silent BOOST_DIR=$MSP_BOOST_DIR
    fi
    make --silent PREFIX=$VIRTUAL_ENV install

    cd ..
    echo "$MSP_LIB installed"
  fi

  # Install the msp430 libraries
  if ! hash msp430-elf-gcc 2>/dev/null; then
    echo "installing mspgcc deps"
    install_deps $1 "stow"
    if [ $? -eq 1 ]; then
      return 1
    fi

    echo "installing mspgcc"
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

    cd $TOOLCHAIN_DIR
    stow -t $VIRTUAL_ENV/bin bin
  fi

  if ! hash mspdebug 2>/dev/null; then
    echo "installing linked mspdebug"
    mspdebug=$VIRTUAL_ENV/bin/mspdebug
    touch $mspdebug
    echo "#!/bin/bash" > $mspdebug
    if [[ "$1" == "linux" ]]; then
      echo "LD_LIBRARY_PATH=\"$VIRTUAL_ENV/lib\" mspdebug_unlinked \"\$@\"" >> $mspdebug
    else
      echo "DYLD_LIBRARY_PATH=$VIRTUAL_ENV/lib mspdebug_unlinked \"\$@\"" >> $mspdebug
    fi
    chmod +x $mspdebug
  fi
}

mkdir -p ".toolchains/ti/mspgcc"
export TOOLCHAIN_DIR=`readlink -f ".toolchains/ti/mspgcc"`

export DIR="tmp_downloads"
mkdir -p $DIR
pushd $DIR >/dev/null

main $@

popd >/dev/null
rm -rf $DIR

unset DIR
unset LINUX_PREFIX
