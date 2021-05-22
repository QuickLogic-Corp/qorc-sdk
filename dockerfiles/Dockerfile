ARG PARENT_IMAGE="docker.pkg.github.com/quicklogic-corp/quicklogic-fpga-toolchain/symbiflow-ql:1.3.0"
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
# Test the container in the /root/qf_apps directory
#
# source ${INSTALL_DIR}/conda/etc/profile.d/conda.sh
# conda activate
# 
# cd /root/qf_apps
# make
#
# When you are finished, you may want to clean the qf_apps directory
# because this container runs as root, which means files are created
# as the root user, and they are difficult to remove as an ordinary user.
#
# make clean
# exit
#
# This container image intentionally does not add a user or entrypoint.
# This allows easy use of this container as a parent image for adapting
# it to specific needs.  An example with a prebuilt ready-to-run Docker
# container package built through a github actions workflow is here:
# https://github.com/Thirsty2/qorc-sdk-user
