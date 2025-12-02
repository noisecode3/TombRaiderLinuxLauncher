#!/bin/bash
cd "$(dirname "$0")" || exit 1
cd build || exit 1
make -j"$(nproc)"
