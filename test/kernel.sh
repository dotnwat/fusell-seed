#!/bin/bash
set -e
set -x

VERSION=4.18.5

curl -O https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-${VERSION}.tar.xz
tar xf linux-${VERSION}.tar.xz

DIR=linux-${VERSION}
pushd ${DIR}

make tinyconfig
make -j$(nproc)
make distclean

popd
rm -rf $DIR
rm linux-${VERSION}.tar.xz
