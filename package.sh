#!/bin/sh

DIR="`tla pristines | cut -f 2 -d /`"

for i in generator.exe linux-i386-generator macosx-generator.dmg generatorUI.cxx generatorUI.h; do
  if [ ! -f "$i" ]; then
    echo "$i is missing"
    exit 1
  fi
done

rm -rf "$DIR"
mkdir -p "$DIR"
cp -pPR *.cxx *.h *.fl linux-i386-generator macosx-generator.dmg generator.exe Dependencies Makefile Fl_Gel_Tabs "$DIR"
find "$DIR" \( -name .arch-ids -or -name '*.o' \) -exec rm -rf '{}' \; 2>/dev/null
tar -czf "$DIR.tar.gz" "$DIR"
