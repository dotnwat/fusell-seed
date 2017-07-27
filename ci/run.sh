#!/bin/bash

set -e
set -x

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJ_DIR=${THIS_DIR}/../

echo user_allow_other | sudo tee /etc/fuse.conf

tmpdir=`mktemp -d 2>/dev/null || mktemp -d -t 'tmpdir'`
./main -o fsname=fusell -o allow_other -o atomic_o_trunc ${tmpdir} &
sleep 5
mount

# tests aren't run on mac
if [[ "$OSTYPE" == "darwin"* ]]; then
  exit 0
fi

pushd ${tmpdir}
sudo ${PROJ_DIR}/test/posix.sh
popd
rm -rf ${tmpdir}
