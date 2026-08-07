#!/bin/sh

. /etc/os-release
OS_VERSION="${ID}${VERSION}"

make -j6 -l8
