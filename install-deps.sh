#!/bin/bash
set -e

SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if test $(id -u) != 0 ; then
  SUDO=sudo
fi

function debs() {
  local tmp=$(mktemp -d)
  trap "rm -rf $tmp" EXIT

  $SUDO apt-get update

  $SUDO env DEBIAN_FRONTEND=noninteractive \
    apt-get install -y devscripts equivs git

  # run mk-build-deps in tmp dir to avoid creation of artifact files that
  # cause errors for read-only docker mounts
  pushd $tmp
  $SUDO env DEBIAN_FRONTEND=noninteractive \
    mk-build-deps --install --remove \
    --tool="apt-get -y --no-install-recommends" \
    ${SRC_DIR}/debian/control || exit 1
  popd
  rm -rf $tmp

  $SUDO env DEBIAN_FRONTEND=noninteractive \
    apt-get -y remove fusell-seed-build-deps
}

source /etc/os-release
case $ID in
  debian|ubuntu)
    debs
    ;;

  *)
    echo "$ID not supported. Install dependencies manually."
    exit 1
    ;;
esac
