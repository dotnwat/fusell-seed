#!/bin/bash

set -e
set -x

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FSTEST_DIR=${THIS_DIR}/pjd-fstest-20090130-RC

pushd $FSTEST_DIR
make
popd

mkdir fstest
pushd fstest
prove -v -r ${FSTEST_DIR}/tests/
popd
rm -rf fstest
