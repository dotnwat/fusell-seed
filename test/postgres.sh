#!/bin/bash
set -e
set -x

VERSION=10.5

curl -O https://ftp.postgresql.org/pub/source/v${VERSION}/postgresql-${VERSION}.tar.gz
tar xzf postgresql-${VERSION}.tar.gz

DIR=postgresql-${VERSION}
pushd ${DIR}

./configure
make -j$(nproc)
make check

popd
rm -rf ${DIR}
rm postgresql-${VERSION}.tar.gz
