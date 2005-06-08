#!/bin/sh

DIR="`tla pristines | cut -f 2 -d /`"

rm -rf "$DIR"
mkdir -p "$DIR"
cp -pPR *.cxx *.h *.fl generator.exe Dependencies Makefile Fl_Gel_Tabs "$DIR"
find "$DIR" -name .arch-ids -exec rm -rf '{}' \; 2>/dev/null
tar -czf "$DIR.tar.gz" "$DIR"
