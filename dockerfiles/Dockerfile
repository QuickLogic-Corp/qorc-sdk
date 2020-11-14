ARG PARENT_IMAGE="symbiflow-ql:latest"
FROM ${PARENT_IMAGE}

WORKDIR /opt

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 && \
    tar -xjvf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 && \
    rm gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2

ENV PATH=/opt/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux:$PATH

# Below are some example commands to build and run a docker container.
# From within the dockerfiles directory in a terminal window type:
#
# docker build . -t qorc-sdk
# 
# In a directory containing a qorc-sdk project:
# docker run -it --rm -v $(pwd):/root qorc-sdk bash
#
# The above will execute bash within the running container.  Any
# tools called thru bash will run within the container.
#
# This container image intentionally does not add a user or entrypoint.
# This allows easy use of this container as a parent image for adapting
# it to specific needs.  An example with a prebuilt ready-to-run Docker
# container package built through a github actions workflow is here:
# https://github.com/Thirsty2/qorc-sdk-user
