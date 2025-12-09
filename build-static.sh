#!/bin/bash

### NONE OF THIS MATTERS UNLESS YOU EDIT ESQLC.MK and change:
#
#ESQL     = INFORMIXC="${CC}" ${ESQL_CMD}
to
#ESQL     = INFORMIXC="${CC} -static -O3" ${ESQL_CMD}

## !!!


SDK="/opt/ifx.sdk.450.fc11"
export INFORMIXDIR="$SDK"
export PATH=${SDK}/bin:$PATH
export 
export CFLAGS="-static -O3"
export LDFLAGS="-static"
export INFORMIXC="gcc -static"
export THREADLIB=posix
echo CFLAGS=$CFLAGS
echo LDFLAGS=$LDFLAGS
make clean && ./configure --prefix=/opt/sqlcmd/ --with-informixdir=${SDK}/ && make -j12
