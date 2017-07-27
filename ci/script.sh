#!/bin/bash

set -e
set -x

THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJ_DIR=${THIS_DIR}/../

if [ ! -z ${DOCKER_IMAGE+x} ]; then
  docker run --privileged --rm \
    -v ${PROJ_DIR}:/code -w="/code" \
    ${DOCKER_IMAGE} /bin/bash -c "./ci/install-deps.sh && ./ci/build.sh"
else
  ${PROJ_DIR}/ci/install-deps.sh
  ${PROJ_DIR}/ci/build.sh
  ${PROJ_DIR}/ci/run.sh
fi
