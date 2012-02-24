#! /bin/sh
mkdir -p build

if [ -d Python ] ; then
    mkdir -p build/python
    pushd build/python
    ./configure --disable-shared --prefix=`pwd`/../../local
    make -j4 && make install
fi
