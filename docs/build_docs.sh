#!/bin/bash

# to build docs, currently we need to do
# git clone https://github.com/QuickLogic-Corp/qorc-sdk.git (NOT recursive)
# cd qorc-sdk
# git checkout qorc-sdk-documentation
# cp -R ./qf_vr_apps/images docs/source/qorc-sdk/qorc-sdk-qf-vr-apps/
#
# git clone https://github.com/QuickLogic-Corp/quick-feather-dev-board.git
# cp -R ./quick-feather-dev-board/img docs/source/ql-development-boards/quickfeather/
#
# git clone https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application.git
# cd TinyFPGA-Programmer-Application/q-series/python
# pandoc --from gfm --to rst -o README.rst readme.md
# or
# git checkout qorc-sdk-documentation
#
# git clone https://github.com/QuickLogic-Corp/qorc-testapps.git
# cd qorc-testapps
# git checkout qorc-sdk-documentation
# cd ..
#
# git clone https://github.com/QuickLogic-Corp/qorc-example-apps.git
# cd qorc-example-apps
# git checkout qorc-sdk-documentation 
# cd ..
# cp -R ./qorc-example-apps/qf_pm2dot5aqi/images docs/source/qorc-sdk/qorc-example-apps/
#
# git clone https://github.com/QuickLogic-Corp/s3-gateware.git
# cd s3-gateware
# git checkout qorc-sdk-documentation
# cd ..
#
# cd docs
# make clean
# make html
# docs/build contains the docs.