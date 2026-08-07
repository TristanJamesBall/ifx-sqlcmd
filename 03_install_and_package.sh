#!/bin/ksh
ME="$( realpath "$0" )"

. /etc/os-release
OS_VERSION="${ID}${VERSION}"
ARCH=$(uname -m)
BUILD_DIR=$(pwd)

TARBALL=${BUILD_DIR}/sqlcmd.${OS_VERSION}.${ARCH}.tgz

if [[ $(whoami) != "root" ]]; then
	exec sudo $ME "$@" || { echo "FAILED TO SUDO" ; exit 1 ;}
fi

umask 000


make install



( cd /opt/informix; tar -czf $TARBALL sqlcmd )
chown informix:informix $TARBALL
aws s3 cp $TARBALL s3://ob-informix-tst-scratch-deiynxxxyanv/transfer/

tput clear
echo
echo
echo "Package is: "
echo
echo -e "\t$TARBALL"
echo 
echo "Uploaded to:"
echo 
echo -e "\ts3://ob-informix-tst-scratch-deiynxxxyanv/transfer/$( basename $TARBALL )"
echo
echo
echo "Please manually copy this to:"
echo
echo -e "\ts3://sb-openbet-builds-i6w0a40xjrstbble/software/$( basename $TARBALL )"
echo
echo "EG:"
echo
echo -e "\taws s3 cp s3://ob-informix-tst-scratch-deiynxxxyanv/transfer/$( basename $TARBALL ) s3://sb-openbet-builds-i6w0a40xjrstbble/software/$( basename $TARBALL )"
