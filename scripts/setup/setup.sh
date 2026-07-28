#!/bin/bash

# Arch-compatible revision.
#
# Changes from the Debian/macOS original:
#   - boost detection no longer uses dpkg; BOOST_ROOT is configurable so an
#     older boost (1.85) can live in a local prefix alongside the system one
#   - added C/C++ standard pins, because GCC >= 15 defaults to C23/C++17+ and
#     rejects code that older GCC accepted
#   - LINUX_PREFIX is validated instead of silently defaulting to /usr/local
#   - stow is taken from the system if present (the old check never matched)
#   - wget/make failures now stop the run instead of cascading
#   - tmp_downloads is kept when a build fails, so you can inspect it
#
# BOOST_ROOT default assumes:
#   cd ~/opt/boost_1_85_0 && ./bootstrap.sh --prefix=$HOME/opt/boost-1.85 \
#     && ./b2 -j$(nproc) --with-system --with-thread --with-filesystem \
#          --with-date-time --with-chrono --with-atomic install
# Override with:  BOOST_ROOT=/some/other/prefix ./setup.sh linux

: "${BOOST_ROOT:=$HOME/opt/boost-1.85}"

# C standard pin for mspdebug (dlbeer) only. GCC >= 15 defaults to C23, which
# turns implicit declarations into hard errors.
: "${MSP_CSTD:=-std=gnu17}"
# NOTE: no CXXSTD pin for mspds -- its makefile sets its own CXXFLAGS
# (-fPIC -std=c++0x ...) and overriding that on the command line drops -fPIC.
# To change the C++ standard there, edit line 1 of scripts/setup/mspds_makefile.

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

verify_prefix() {
  if [ -z "$VIRTUAL_ENV" ]; then
    echo "ERROR: no virtualenv is active."
    echo "This script installs into \$VIRTUAL_ENV; without it, 'make install'"
    echo "would write to /usr/local. Activate the project venv and re-run."
    return 1
  fi
  if [ ! -d "$VIRTUAL_ENV" ]; then
    echo "ERROR: \$VIRTUAL_ENV points at $VIRTUAL_ENV, which does not exist."
    return 1
  fi
  echo "Installing into prefix: $VIRTUAL_ENV"
  echo "Python: $(python --version 2>&1)"
}

export LINUX_PREFIX=$VIRTUAL_ENV

linux_install() {
  if [[ "$1" == "hidapi" ]]; then
    if [ -d "$LINUX_PREFIX/include/hidapi" ] ; then
      echo "hidapi already installed!"
      return 0
    fi

    export PKG_CONFIG_PATH=$LINUX_PREFIX/lib/pkgconfig/
    git clone https://github.com/libusb/hidapi.git || return 1
    cd hidapi || return 1
    ./bootstrap || return 1
    ./configure --prefix=$LINUX_PREFIX || return 1
    make || return 1
    make install || return 1
    cd ..
  elif [[ "$1" == "libusb-compat" ]]; then
    if [ -f "$LINUX_PREFIX/include/usb.h" ] ; then
      echo "libusb-compat already installed!"
      return 0
    fi

    git clone https://github.com/libusb/libusb.git || return 1
    cd libusb || return 1
    ./autogen.sh || return 1
    ./configure --prefix=$LINUX_PREFIX || return 1
    make || return 1
    make install || return 1
    cd ..

    export PKG_CONFIG_PATH=$LINUX_PREFIX/lib/pkgconfig/
    git clone https://github.com/libusb/libusb-compat-0.1.git || return 1
    cd libusb-compat-0.1 || return 1
    ./autogen.sh || return 1
    ./configure --prefix=$LINUX_PREFIX || return 1
    make || return 1
    make install || return 1
    cd ..
  elif [[ "$1" == "boost" ]]; then
    # Prefer a pinned boost in BOOST_ROOT; fall back to the system one.
    # Checking for the header is distro-agnostic, unlike 'dpkg -s'.
    if [ -f "$BOOST_ROOT/include/boost/version.hpp" ]; then
      # The Linux branch of mspds_makefile reads three separate variables:
      #   BOOST_DIR     -- only an ifdef guard, value unused
      #   BOOST_INCLUDE -> -I  (parent of boost/, not boost/ itself)
      #   BOOST_LIB     -> -L  (leaving this unset yields a bare -L)
      export MSP_BOOST_DIR="$BOOST_ROOT"
      export MSP_BOOST_INCLUDE="$BOOST_ROOT/include"
      export MSP_BOOST_LIB="$BOOST_ROOT/lib"
      # A prefix under $HOME is not in the default loader path. LD_RUN_PATH
      # bakes an rpath into libmsp430.so so it resolves boost at runtime;
      # without this the build succeeds and loading fails later.
      export LD_LIBRARY_PATH="$BOOST_ROOT/lib:$LD_LIBRARY_PATH"
      export LD_RUN_PATH="$BOOST_ROOT/lib:$LD_RUN_PATH"
      # BOOST_INCLUDE/BOOST_LIB only reach the top-level mspds_makefile. The
      # TI tree recurses into its own sub-makefiles (BSL430_DLL -> libbsl430.a)
      # which ignore those variables and pick up /usr/include/boost instead --
      # fatal once the system boost is >= 1.87 (io_service removed).
      # CPLUS_INCLUDE_PATH / LIBRARY_PATH are read by GCC itself, reach every
      # sub-make, and are searched before /usr/include and /usr/lib, so the
      # pinned 1.85 shadows the system boost everywhere.
      export CPLUS_INCLUDE_PATH="$BOOST_ROOT/include${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
      export LIBRARY_PATH="$BOOST_ROOT/lib${LIBRARY_PATH:+:$LIBRARY_PATH}"
      echo "using pinned boost at $BOOST_ROOT"
      echo "  $(grep BOOST_LIB_VERSION "$BOOST_ROOT/include/boost/version.hpp")"
      return 0
    elif [ -f /usr/include/boost/version.hpp ]; then
      export MSP_BOOST_DIR=/usr
      export MSP_BOOST_INCLUDE=/usr/include
      export MSP_BOOST_LIB=/usr/lib
      echo "WARNING: falling back to system boost."
      echo "  $(grep BOOST_LIB_VERSION /usr/include/boost/version.hpp)"
      echo "  MSPDebugStack needs boost < 1.87 (boost::asio::io_service was"
      echo "  removed). If the build fails on io_service, build 1.85 into"
      echo "  $BOOST_ROOT and re-run."
      return 0
    else
      echo "Please install boost (pacman -S boost boost-libs), or build 1.85"
      echo "into $BOOST_ROOT"
      return 1
    fi
  elif [[ "$1" == "stow" ]]; then
    # The original checked for $LINUX_PREFIX/include/stow, which never exists:
    # stow is a Perl script and installs no headers, so it rebuilt every run.
    if command -v stow >/dev/null 2>&1; then
      echo "stow already available: $(command -v stow)"
      return 0
    fi
    if [ -x "$LINUX_PREFIX/bin/stow" ]; then
      echo "stow already installed in prefix!"
      return 0
    fi

    echo "stow not found. Install it with: sudo pacman -S stow"
    echo "(building it from git needs autoconf/automake/texinfo and perl deps)"
    return 1
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
        BREW_INSTALL+=" $d"   # was missing the separator
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

  if [[ "$1" == "linux" ]]; then
    verify_prefix || return 1
  fi

  ## Install mspdebug
  # Check the prefix, not PATH: you have a system-wide mspdebug from the AUR,
  # and a PATH hit there would silently skip these blocks.
  if [ ! -x "$VIRTUAL_ENV/bin/mspdebug_unlinked" ]; then
    echo "installing mspdebug deps"
    install_deps $1 "libusb-compat" "hidapi" 
    if [ $? -eq 1 ]; then
      return 1
    fi

    echo "installing mspdebug"
    git clone https://github.com/dlbeer/mspdebug || return 1
    cd mspdebug || return 1

    # hidapi fix for mspdebug
    if [[ "$1" == "linux" ]]; then
      export PKG_CONFIG_PATH=$LINUX_PREFIX/lib/pkgconfig
      # MSP_CSTD matters on GCC >= 15: the C23 default turns implicit
      # declarations and int/pointer conversions into errors.
      export CFLAGS="$MSP_CSTD $(pkg-config --cflags libusb hidapi-libusb)"
      LIB_FLAGS=$(pkg-config --libs libusb hidapi-libusb)
      sed -i "s|-lusb| $LIB_FLAGS|g" Makefile
    else
      export CFLAGS="-I/opt/homebrew/include"
    fi

    # make
    make --silent PREFIX=$VIRTUAL_ENV install || return 1
    mv $VIRTUAL_ENV/bin/mspdebug $VIRTUAL_ENV/bin/mspdebug_unlinked || return 1

    cd ..
    echo "mspdebug_unlinked installed"
  fi

  if [[ "$1" == "linux" ]]; then
    MSP_LIB=libmsp430.so
    HID_PATH=$LINUX_PREFIX/include/hidapi/hidapi.h
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
    wget "https://dr-download.ti.com/software-development/driver-or-library/MD-4vnqcP1Wk4/3.15.1.1/MSPDebugStack_OS_Package_3_15_1_1.zip" || return 1
    unzip -q "MSPDebugStack_OS_Package_3_15_1_1.zip" -d mspds || return 1
    cd mspds || return 1

    cp $HID_PATH ./ThirdParty/include/ || return 1
    
    cp ../../scripts/setup/mspds_makefile ./Makefile || return 1

    if [[ "$1" == "linux" ]]; then
      export PKG_CONFIG_PATH=$LINUX_PREFIX/lib/pkgconfig/
      # Do NOT pass CXXFLAGS= here. A command-line assignment replaces the
      # makefile's own line 1 (-fPIC -std=c++0x -fvisibility=hidden ...)
      # outright, and losing -fPIC breaks the -shared link with a TPOFF32
      # relocation error. BOOST_INCLUDE already feeds INCLUDES, so no -I
      # is needed here either.
      make --silent \
        BOOST_DIR=$MSP_BOOST_DIR \
        BOOST_INCLUDE=$MSP_BOOST_INCLUDE \
        BOOST_LIB=$MSP_BOOST_LIB || return 1
    else
      export PKG_CONFIG_PATH=/opt/homebrew/lib/pkgconfig/
      make --silent BOOST_DIR=$MSP_BOOST_DIR || return 1
    fi
    make --silent PREFIX=$VIRTUAL_ENV install || return 1

    cd ..
    echo "$MSP_LIB installed"
  fi

  # Install the msp430 libraries
  if [ ! -x "$VIRTUAL_ENV/bin/msp430-elf-gcc" ]; then
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

    wget $GCC_URL || return 1
    wget $GCC_SUP_URL || return 1

    tar xjf "$GCC_DIRNAME.tar.bz2" || return 1
    unzip -qx msp430-gcc-support-files-1.212.zip || return 1
    mv msp430-gcc-support-files/include/* $GCC_DIRNAME/include || return 1

    # Refuse to merge into a populated toolchain dir: the old contents are
    # from a previous build and 'mv' would half-overwrite them.
    if [ -n "$(ls -A "$TOOLCHAIN_DIR" 2>/dev/null)" ]; then
      echo "ERROR: $TOOLCHAIN_DIR is not empty."
      echo "For a clean rebuild: rm -rf .toolchains"
      return 1
    fi
    mkdir -p $TOOLCHAIN_DIR || return 1
    mv $GCC_DIRNAME/* $TOOLCHAIN_DIR || return 1

    cd $TOOLCHAIN_DIR || return 1
    stow -t $VIRTUAL_ENV/bin bin || return 1
    cd - >/dev/null
  fi

  if [ ! -x "$VIRTUAL_ENV/bin/mspdebug" ]; then
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

# Are we being sourced, or executed? If sourced, 'exit' would terminate the
# caller's shell -- which over SSH means the connection drops.
_SETUP_SOURCED=0
if [ "${BASH_SOURCE[0]}" != "${0}" ]; then
  _SETUP_SOURCED=1
fi

mkdir -p ".toolchains/ti/mspgcc"
export TOOLCHAIN_DIR=`readlink -f ".toolchains/ti/mspgcc"`

export DIR="tmp_downloads"
mkdir -p $DIR
pushd $DIR >/dev/null

main $@
MAIN_RC=$?

popd >/dev/null

# Keep the scratch dir on failure so the build tree can be inspected.
if [ $MAIN_RC -eq 0 ]; then
  rm -rf $DIR
else
  echo "setup failed (rc=$MAIN_RC); leaving $DIR in place for inspection"
fi

# Build vars leak into the calling shell when sourced; CFLAGS in particular
# will quietly break unrelated builds in this session.
unset DIR
unset LINUX_PREFIX
unset CFLAGS
unset PKG_CONFIG_PATH
unset CPLUS_INCLUDE_PATH
unset LIBRARY_PATH

if [ "$_SETUP_SOURCED" -eq 1 ]; then
  unset _SETUP_SOURCED
  # 'return' at top level is only valid in a sourced script.
  return $MAIN_RC
else
  unset _SETUP_SOURCED
  exit $MAIN_RC
fi
