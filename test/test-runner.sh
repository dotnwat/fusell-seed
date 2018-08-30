#!/bin/bash
set -e
set -x

fs=${1}
script=${2}
size=1610612736
dir=$(mktemp -d)

${fs} -o size=${size} ${dir} &
pid=$!

function cleanup() {
  fusermount -u ${dir} || true
  kill ${pid} || true
}

trap "cleanup" EXIT

retries=5
until findmnt -M ${dir} -t fuse.main; do
  retries=$((${retries} - 1))
  [[ ${retries} -gt 0 ]] || exit 1
  echo "waiting... ${retries}"
  sleep 1
done

# TODO add some stats checks that the fs is empty after cleanup
pushd ${dir}
${script}
popd
