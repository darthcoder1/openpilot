#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

DEP_TO_INSTALL=$1

# install tensorflow

if [[ -z "$DEP_TO_INSTALL" || "$DEP_TO_INSTALL" == "tensorflow" ]]; then
    TENSORFLOW_FILENAME=libtensorflow-gpu-linux-x86_64-1.14.0.tar.gz
    mkdir tensorflow
    wget https://storage.googleapis.com/tensorflow/libtensorflow/$TENSORFLOW_FILENAME

    tar -C tensorflow/ -xzf $TENSORFLOW_FILENAME
    rm $TENSORFLOW_FILENAME
fi

if [[ -z "$DEP_TO_INSTALL" || "$DEP_TO_INSTALL" == "libhybris" ]]; then
    # install android-headers (needed by libhybris)
    apt install android-headers

    echo "$SCRIPT_DIR/libhybris"
    if [ ! -d "$SCRIPT_DIR/libhybris" ]; then
        git clone git@github.com:ubports/libhybris.git
    fi

    cd $SCRIPT_DIR/libhybris/hybris/

    # generate the makefiles
    CPPFLAGS=-I/usr/include/android-19/ ./autogen.sh
fi
