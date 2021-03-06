#!/bin/sh

REV=`hg tip --template={rev}`
DIR="generator-fltk-r$REV"

for i in generator.exe linux-i386-generator macosx-generator generatorUI.cxx generatorUI.h; do
  if [ ! -f "$i" ]; then
    echo "$i is missing"
    exit 1
  fi
done

rm -rf "$DIR"
mkdir -p "$DIR"
cp -pPR *.c *.cxx *.h *.fl *.rc *.ico *.xpm linux-i386-generator macosx-generator generator.exe Dependencies Makefile Fl_Gel_Tabs FLU libmd libbz2 debian "$DIR"
find "$DIR" \( -name '*.o' \) -exec rm -rf '{}' \; 2>/dev/null
tar -czf "$DIR.tar.gz" "$DIR"
