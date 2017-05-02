#!/bin/bash

set -e
set -x

if [ ! -z ${DOCKER_IMAGE+x} ]; then
  ci_env=""
  if [ "${RUN_COVERAGE}" == 1 ]; then
    ci_env=`bash <(curl -s https://codecov.io/env)`
  fi
  docker run --net=host --rm -v \
    ${TRAVIS_BUILD_DIR}:/code -w="/code" \
    $ci_env -e RUN_COVERAGE=${RUN_COVERAGE} \
    ${DOCKER_IMAGE} /bin/bash -c "./ci/install-deps.sh && ./ci/run.sh"
else
  ${TRAVIS_BUILD_DIR}/ci/install-deps.sh
  ${TRAVIS_BUILD_DIR}/ci/run.sh
fi
