#!/bin/bash
set -e
set -x

fs=${1}
script=${2}
size=1073741824
dir=$(mktemp -d)

${fs} -o size=${size} ${dir} &
pid=$!

until findmnt -M ${dir} -t fuse.main; do
  echo "waiting..."
done

pushd ${dir}
${script}
popd

kill ${pid}
