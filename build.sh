#!/bin/sh

# Path to fluid needed to compile generator
FLUID=fluid

# Path to GNU make
MAKE=gmake

# Compress using upx the output binary (comment UPX to disable)
UPX=upx
UPXFLAGS="--best --crp-ms=999999 --nrv2b"

# Those aren't really used, just need to have them diffrent to enable cross.
CROSS_TARGET=i386-pc-mingw32
CROSS_HOST=i386-pc-linux

# `uname` expected output by target cross (needed for fltk)
UNAME=MINGW

# Output binary extension
EXEEXT=.exe

# Cross tools prefix
CROSS_PREFIX=mingw32
#CROSS_PREFIX=i586-mingw32msvc

# FLTK version used
#FLTKVER=1.1.x-r4393
#FLTKURL=http://ftp.easysw.com/pub/fltk/snapshots/fltk-$FLTKVER.tar.bz2
FLTKVER=1.1.6
FLTKURL=http://ftp.easysw.com/pub/fltk/$FLTKVER/fltk-$FLTKVER-source.tar.bz2

# Paths to mkzftree and mkisofs
MKZFTREE='win32\\\\mkzftree.exe'
MKISOFS='win32\\\\mkisofs.exe'

export EXEEXT
export CFLAGS="-Os"
export CXXFLAGS="-Os"
export CC=$CROSS_PREFIX-gcc
export CXX=$CROSS_PREFIX-g++
export AR=$CROSS_PREFIX-ar
export LD=$CROSS_PREFIX-ld
export STRIP=$CROSS_PREFIX-strip
export RANLIB=$CROSS_PREFIX-ranlib

TMPDIR=tmp
FLTKDIR=$TMPDIR/fltk-$FLTKVER

set -e

export ac_cv_path_AR=$AR

[ ! -d $TMPDIR ] && mkdir $TMPDIR

WORKDIR=$PWD

if [ ! -f $TMPDIR/.get ]; then
  wget --passive-ftp -c -P $TMPDIR $FLTKURL
  touch $TMPDIR/.get
  rm -f $TMPDIR/.extract $TMPDIR/.patch $TMPDIR/.configure $TMPDIR/.build
fi

if [ ! -f $TMPDIR/.extract ]; then
  rm -rf $FLTKDIR
  tar -xjf $FLTKDIR*.tar.bz2 -C $TMPDIR
  touch $TMPDIR/.extract
  rm -f $TMPDIR/.patch $TMPDIR/.configure $TMPDIR/.build
fi

if [ ! -f $TMPDIR/.patch ]; then
  cd $FLTKDIR
  sed 's%uname=.*%uname=CYGWIN%g' configure > configure.new
  mv configure.new configure
  chmod +x configure
  sed 's%ShellApi.h%shellapi.h%' src/Fl_win32.cxx > src/Fl_win32.cxx.new
  mv src/Fl_win32.cxx.new src/Fl_win32.cxx
  cd $WORKDIR
  touch $TMPDIR/.patch
  rm -f $TMPDIR/.configure $TMPDIR/.build
fi

if [ ! -f $TMPDIR/.configure ]; then
  cd $FLTKDIR
  ./configure \
              --host=$CROSS_TARGET \
              --build=$CROSS_HOST \
              --disable-localjpeg \
              --disable-localzlib \
              --disable-localpng \
              --disable-gl 
  cd $WORKDIR
  touch $TMPDIR/.configure
  rm -f $TMPDIR/.build
fi

if [ ! -f $TMPDIR/.build ]; then
  cd $FLTKDIR/src
  $MAKE
  cd $WORKDIR
  touch $TMPDIR/.build
  make clean
fi

export FLTKCXXFLAGS="`$FLTKDIR/fltk-config --cxxflags | sed s%/usr/local/include%$FLTKDIR%g`"
export FLTKLDFLAGS="`$FLTKDIR/fltk-config --ldstaticflags | sed s%/usr/local%$FLTKDIR%g`"
export CXXFLAGS="$CXXFLAGS -DPATH_MKZFTREE=\\\"$MKZFTREE\\\" -DPATH_MKISOFS=\\\"$MKISOFS\\\""

$MAKE

if [ -n $UPX ]; then
    $UPX $UPXFLAGS generator$EXEEXT
fi

echo "DONE"
