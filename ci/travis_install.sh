#!/bin/bash
set -xe #Exit on failure.

STAMP="+[%H:%M:%S]"
date "$STAMP"

cd "${TRAVIS_BUILD_DIR}"

if [[ "$(uname -s)" == 'Darwin' ]]; then
    sysctl -n machdep.cpu.brand_string
    sysctl machdep.cpu.family
    sysctl -n machdep.cpu.features
    sysctl -n machdep.cpu.leaf7_features
    sysctl -n machdep.cpu.extfeatures

    set -x
    brew update || brew update
    date "$STAMP"
    brew install nasm
    ./ci/nixtools/install_dssim.sh
    set +x
else
    cat /proc/cpuinfo
    date "$STAMP"
    set -x
    docker pull "${DOCKER_IMAGE}"
    set +x
fi

