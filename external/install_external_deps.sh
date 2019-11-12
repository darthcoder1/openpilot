#!/bin/bash

# install tensorflow
TENSORFLOW_FILENAME=libtensorflow-gpu-linux-x86_64-1.14.0.tar.gz
mkdir tensorflow
wget https://storage.googleapis.com/tensorflow/libtensorflow/$TENSORFLOW_FILENAME

tar -C tensorflow/ -xzf $TENSORFLOW_FILENAME
rm $TENSORFLOW_FILENAME

# install mesa
