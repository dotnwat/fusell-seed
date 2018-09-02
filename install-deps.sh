#!/bin/bash
set -e

if test $(id -u) != 0 ; then
  SUDO=sudo
fi

function debs() {
  $SUDO apt-get update
  $SUDO apt-get install -y \
    cmake \
    pkg-config \
    libfuse-dev
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
