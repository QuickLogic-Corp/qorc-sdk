#!/bin/bash
set -e

source ${INSTALL_DIR}/conda/etc/profile.d/conda.sh
conda activate

pip3 install tinyfpgab pyserial

exec "$@"

conda deactivate

