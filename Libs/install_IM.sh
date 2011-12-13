#!/bin/bash

installPath="ImageMagick-6.6.9"
libpngPath="libpng-1.2.46"
imPath="ImageMagick-6.6.9-0"

rm -rf $imPath
rm -rf $libpngPath
rm -rf $installPath

tar -xf $imPath.tar.gz
tar -xf $libpngPath.tar.gz

currPath=`pwd`
cd $libpngPath
./configure --prefix=$currPath/$installPath/delegates --enable-static --disable-shared
make
make install
cp $currPath/$installPath/delegates/lib/libpng.a $currPath/$installPath/delegates/lib/libpng-static.a
cd ../$imPath

./configure --prefix=$currPath/$installPath --disable-shared --without-dps --without-djvu --without-fontconfig --without-freetype --without-gslib --without-gvc --without-lcms --without-lcms2 --without-lqr --without-lzma --without-openexr --without-rsvg --without-webp --without-wmf --without-xml --disable-openmp --disable-opencl --with-x=no CPPFLAGS=-I$currPath/$installPath/delegates/include LDFLAGS=-L$currPath/$installPath/delegates/lib
make
make install
cd ..

rm -rf $libpngPath
rm -rf $imPath

