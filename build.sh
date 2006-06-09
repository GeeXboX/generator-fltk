#!/bin/sh

# Path to fluid needed to compile generator
FLUID=fluid

# Path to GNU make
MAKE="`which gmake make | head -1`"

# Compress using upx the output binary (comment UPX to disable)
UPX=upx
UPXFLAGS="--brute"
UPX_VERSION="1.95"

# Those aren't really used, just need to have them diffrent to enable cross.
CROSS_TARGET=i386-pc-mingw32
CROSS_HOST=i386-pc-linux

# `uname` expected output by target cross (needed for fltk)
UNAME=MINGW

# Output binary extension
EXEEXT=.exe

# Cross tools prefix
for i in mingw32 i586-mingw32msvc; do
  which "$i-g++" >/dev/null 2>&1 && CROSS_PREFIX=$i && break
done
#CROSS_PREFIX=mingw32

# FLTK version used
FLTKVER=1.1.x-r5187
FLTKURL=http://ftp.easysw.com/pub/fltk/snapshots/fltk-$FLTKVER.tar.bz2
#FLTKVER=1.1.7
#FLTKURL=http://ftp.easysw.com/pub/fltk/$FLTKVER/fltk-$FLTKVER-source.tar.bz2

# cURL version used
CURLVER=7.15.3
CURLURL=http://curl.haxx.se/download/curl-$CURLVER.tar.bz2

export EXEEXT
export CFLAGS="-Os -fomit-frame-pointer"
export CXXFLAGS="$CFLAGS"
export CC=$CROSS_PREFIX-gcc
export CXX=$CROSS_PREFIX-g++
export AR=$CROSS_PREFIX-ar
export LD=$CROSS_PREFIX-ld
export STRIP=$CROSS_PREFIX-strip
export RANLIB=$CROSS_PREFIX-ranlib
export OBJDUMP=$CROSS_PREFIX-objdump
export OBJCOPY=$CROSS_PREFIX-objcopy
export DLLTOOL=$CROSS_PREFIX-dlltool
export F77=$CROSS_PREFIX-g77
export AS=$CROSS_PREFIX-as
export NM=$CROSS_PREFIX-nm
export CPP=$CROSS_PREFIX-cpp

TMPDIR=tmp
FLTKDIR=$TMPDIR/fltk-$FLTKVER
CURLDIR=$TMPDIR/curl-$CURLVER

set -e

export ac_cv_path_AR=$AR

[ ! -d $TMPDIR ] && mkdir $TMPDIR

WORKDIR=$PWD
INSTDIR=$PWD/$TMPDIR/target

if [ ! -f $TMPDIR/.get ]; then
  wget --passive-ftp -c -P $TMPDIR $FLTKURL
  wget --passive-ftp -c -P $TMPDIR $CURLURL
  touch $TMPDIR/.get
  rm -f $TMPDIR/.extract $TMPDIR/.patch $TMPDIR/.configure $TMPDIR/.build $TMPDIR/.install
fi

if [ ! -f $TMPDIR/.extract ]; then
  rm -rf $FLTKDIR
  tar -xjf $FLTKDIR*.tar.bz2 -C $TMPDIR
  rm -rf $CURLDIR
  tar -xjf $CURLDIR*.tar.bz2 -C $TMPDIR
  touch $TMPDIR/.extract
  rm -f $TMPDIR/.patch $TMPDIR/.configure $TMPDIR/.build $TMPDIR/.install
fi

if [ ! -f $TMPDIR/.patch ]; then
  cd $FLTKDIR
  sed -e "s%uname=.*%uname=$UNAME%g" -e 's%-lwsock32%-lws2_32%' configure > configure.new
  mv configure.new configure
  chmod +x configure
  for i in src/Fl_win32.cxx src/fl_dnd_win32.cxx; do
    sed -e 's%winsock.h%winsock2.h%g' $i > $i.new
    mv $i.new $i
  done
  cd $WORKDIR
  touch $TMPDIR/.patch
  rm -f $TMPDIR/.configure $TMPDIR/.build $TMPDIR/.install
fi

if [ ! -f $TMPDIR/.configure ]; then
  cd $FLTKDIR
  ./configure \
              --host=$CROSS_TARGET \
              --build=$CROSS_HOST \
              --prefix=$INSTDIR \
              --without-links \
              --enable-largefile \
              --disable-localjpeg \
              --disable-localzlib \
              --disable-localpng \
              --disable-gl 
  cd $WORKDIR
  cd $CURLDIR
  ./configure \
              --host=$CROSS_TARGET \
              --build=$CROSS_HOST \
              --prefix=$INSTDIR \
              --disable-shared \
              --enable-static \
              --enable-largefile \
              --enable-http \
              --disable-ftp \
              --disable-gopher \
              --disable-file \
              --disable-ldap \
              --disable-dict \
              --disable-telnet \
              --disable-tftp \
              --disable-manual \
              --disable-ipv6 \
              --enable-nonblocking \
              --disable-ares \
              --enable-verbose \
              --disable-sspi \
              --disable-debug \
              --disable-crypto-auth \
              --disable-cookies \
              --without-ssl \
              --without-gnutls \
              --without-zlib \
              --without-libidn
  cd $WORKDIR
  touch $TMPDIR/.configure
  rm -f $TMPDIR/.build $TMPDIR/.install
fi

if [ ! -f $TMPDIR/.build ]; then
  cd $FLTKDIR/src
  $MAKE
  cd $WORKDIR
  cd $CURLDIR/lib
  $MAKE
  cd $WORKDIR
  touch $TMPDIR/.build
  rm -f $TMPDIR/.install
  make clean
fi

if [ ! -f $TMPDIR/.install ]; then
  rm -rf $INSTDIR
  cd $FLTKDIR
  $MAKE DIRS=src install
  cd $WORKDIR
  cd $CURLDIR/lib
  $MAKE install
  cd ../include
  $MAKE install
  cd ../
  $MAKE install-exec-am
  cd $WORKDIR
  touch $TMPDIR/.install
  make clean
fi

export FLTKCONFIG="$INSTDIR/bin/fltk-config"
export CURLCONFIG="$INSTDIR/bin/curl-config"

$MAKE

if [ -n "$UPX" ]; then
    $UPX $UPXFLAGS generator$EXEEXT || echo "Failed to run upx, requires atleast $UPX_VERSION"
fi

echo "DONE"
