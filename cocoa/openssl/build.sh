#!/bin/bash

OPENSSL_VERSION="1.0.2g"
CWD=`cd $(dirname $0) && pwd`

cd $CWD

rm -rf bin/
mkdir -p bin/
cd bin/

curl -O https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz
tar -xvzf openssl-$OPENSSL_VERSION.tar.gz
mv openssl-$OPENSSL_VERSION openssl_i386
tar -xvzf openssl-$OPENSSL_VERSION.tar.gz
mv openssl-$OPENSSL_VERSION openssl_x86_64
cd openssl_i386
./Configure darwin-i386-cc
make
cd ../
cd openssl_x86_64
./Configure darwin64-x86_64-cc
make

cd $CWD

rm -rf lib/
rm -rf include/
mkdir -p lib/
mkdir -p include/openssl/

lipo \
  bin/openssl_i386/libcrypto.a \
  bin/openssl_x86_64/libcrypto.a \
  -create -output lib/libcrypto.a

lipo \
  bin/openssl_i386/libssl.a \
  bin/openssl_x86_64/libssl.a \
  -create -output lib/libssl.a

cp bin/openssl_i386/include/openssl/* include/openssl/
