#!/bin/bash
set -e

docker run \
    --interactive \
    --tty \
     -v "$(pwd)":/opt/source \
    openpilot /bin/bash
