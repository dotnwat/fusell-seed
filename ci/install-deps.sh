#!/bin/bash

set -e
set -x

if [[ "$OSTYPE" == "darwin"* ]]; then
  brew update || true
  brew tap caskroom/cask || true
  brew cask install osxfuse || true
  exit 0
fi

if test $(id -u) != 0 ; then
  SUDO=sudo
fi

source /etc/os-release
case $ID in
  debian|ubuntu)
    $SUDO apt-get update -qq
    $SUDO apt-get install -qq build-essential pkg-config libfuse-dev
	;;


  centos|fedora)
    yumdnf="yum"
    if command -v dnf > /dev/null; then
      yumdnf="dnf"
    fi
    $SUDO $yumdnf install -y fuse-devel gcc-c++ make pkgconfig
	;;

  *)
    echo "$ID not supported."
    exit 1
    ;;
esac
