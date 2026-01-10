#!/bin/bash

set -e

OPENSSL_VERSION="3.5.4"
CWD=`cd $(dirname $0) && pwd`

cd $CWD

rm -rf bin/
mkdir -p bin/
cd bin/

curl -L -O https://github.com/openssl/openssl/releases/download/openssl-$OPENSSL_VERSION/openssl-$OPENSSL_VERSION.tar.gz
tar -xvzf openssl-$OPENSSL_VERSION.tar.gz
mv openssl-$OPENSSL_VERSION openssl_x86_64
tar -xvzf openssl-$OPENSSL_VERSION.tar.gz
mv openssl-$OPENSSL_VERSION openssl_arm64 

cd openssl_x86_64
./Configure darwin64-x86_64-cc
make
cd ../
cd openssl_arm64
./Configure darwin64-arm64-cc
make

cd $CWD

rm -rf lib/
rm -rf include/
mkdir -p lib/
mkdir -p include/openssl/

lipo \
  bin/openssl_x86_64/libcrypto.a \
  bin/openssl_arm64/libcrypto.a \
  -create -output lib/libcrypto.a

lipo \
  bin/openssl_x86_64/libssl.a \
  bin/openssl_arm64/libssl.a \
  -create -output lib/libssl.a

cp bin/openssl_x86_64/include/openssl/* include/openssl/
