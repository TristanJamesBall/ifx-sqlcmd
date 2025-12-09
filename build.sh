#!/bin/bash

SDK="/opt/ifx.sdk.450.fc11"
export PATH=${SDK}/bin:$PATH
export CFLAGS="-Wl,-rpath=${SDK}/lib:${SDK}/lib/esql/"
./configure --prefix=/opt/informix/sqlcmd/ --with-informixdir=${SDK}/
