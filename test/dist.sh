#!/bin/bash
#

set -xe

eval `grep VERSION= config.status`
DIST=colm-$VERSION.tar.gz

trap "rm -f $DIST" EXIT
make dist

WORKDIR=`mktemp -d /tmp/colm.XXXXXX`
#trap "rm -Rf $DIST $WORKDIR" EXIT
tar -C $WORKDIR -xzvf $DIST

cd $WORKDIR/${DIST%.tar.gz}
./configure && make -j8
cd test && ./runtests
