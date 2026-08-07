#!/bin/bash
set -e

## Because of the weird build system, this only means we staticly link the sdk, not the other OS libraries
## but thats OK, and static linking the SDK is helpful.

export CFLAGS=-static CXXFLAGS=-static LDFLAGS=-static 
. /etc/os-release
OS_VERSION="${ID}${VERSION}"
make clean
./configure --prefix=/opt/informix/sqlcmd --with-informixdir=$INFORMIXDIR
