# Yaak readme

## Run openpilot

## On Ubuntu

First, build the docker image from `Dockerfile.yaak`. This is a modified version
of `Dockerfile.openpilot`.

```
# build the docker image
$ ./build_docker.sh

# run the docker image
$ ./run_docker_shell.sh

# run openpilot
root@docker $ cd /opt/source
root@docker $ ./launch_ubuntu.sh
```
