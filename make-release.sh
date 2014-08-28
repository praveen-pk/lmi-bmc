#!/bin/bash


PKG="openlmi-bmc"

if [ $# -lt 1 ];
then
    printf "Usage: %s GIT-COMMITISH\n(see git describe)\n" $0
    exit 1
fi

VERSION=$(git describe $1 | sed 's/-/_/g')

if [ $? -ne 0 ];
then
    exit 2
fi

if [ -f "$PKG-$VERSION.tar.gz" ];
then
    printf "Archive $PKG-$VERSION.tar.gz already exists\n"
    exit 3
fi

git archive --format=tar --prefix=$PKG-$VERSION/ $1 | gzip > $PKG-$VERSION.tar.gz
res=$?
printf "$PKG-$VERSION.tar.gz\n"
exit $?

