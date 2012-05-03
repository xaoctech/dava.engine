#!/bin/sh 
export SDK=5.0

buildit()
{
target=$1
platform=$2

export CC=/Developer/Platforms/${platform}.platform/Developer/usr/bin/gcc
export CFLAGS="-arch ${target} -isysroot /Developer/Platforms/${platform}.platform/Developer/SDKs/${platform}${SDK}.sdk"
export CPP="/Developer/Platforms/${platform}.platform/Developer/usr/bin/llvm-cpp-4.2"
export AR=/Developer/Platforms/${platform}.platform/Developer/usr/bin/ar
export RANLIB=/Developer/Platforms/${platform}.platform/Developer/usr/bin/ranib

./configure --disable-shared --without-ssl --without-libssh2 --without-ca-bundle --without-ldap --disable-ldap \
--host=${target}-apple-darwin10

make clean
make
$AR rv libcurl.${target}.a lib/*.o
}

buildit armv6 iPhoneOS
buildit armv7 iPhoneOS
buildit i386 iPhoneSimulator

lipo -create libcurl.armv7.a libcurl.armv6.a libcurl.i386.a -output libcurl.a
