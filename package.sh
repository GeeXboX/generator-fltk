#!/bin/sh

DIR="`tla pristines | cut -f 2 -d /`"

rm -rf "$DIR"
mkdir -p "$DIR"
cp *.cxx *.h *.fl generator.exe Dependencies Makefile "$DIR"
tar -czf "$DIR.tar.gz" "$DIR"
