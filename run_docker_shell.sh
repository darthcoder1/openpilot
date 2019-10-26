#!/bin/bash
set -e

docker build -t openpilot -f Dockerfile.openpilot .

docker run \
    --interactive --tty \
     -v "$(pwd)":/opt/source \
    openpilot /bin/bash
