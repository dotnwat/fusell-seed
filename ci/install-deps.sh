#!/bin/bash

set -e
set -x

if [[ "$OSTYPE" == "darwin"* ]]; then
  brew update || true
  brew tap caskroom/cask || true
  brew cask install osxfuse || true
  brew install cmake || true
  exit 0
fi

if test $(id -u) != 0 ; then
  SUDO=sudo
fi

source /etc/os-release
case $ID in
  debian|ubuntu)
    $SUDO apt-get update -qq
    $SUDO apt-get install -qq build-essential pkg-config \
        libfuse-dev libacl1-dev cmake
	;;

  centos|fedora)
    yumdnf="yum"
    if command -v dnf > /dev/null; then
      yumdnf="dnf"
    fi
    $SUDO $yumdnf install -y fuse-devel gcc-c++ make pkgconfig \
        libacl-devel perl-Test-Harness cmake
	;;

  *)
    echo "$ID not supported."
    exit 1
    ;;
esac
