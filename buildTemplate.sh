#!/bin/bash
# This script requires MinGW64 (installation with MSYS2 is preferred)

echo "This is a template script! Please edit me!"
exit 1

basedir="$1"
name="$2"
libs="<your Lua base directory>"

basedir=$(readlink -f "$basedir")
file="${basedir}/${name}"
if [ "$MSYSTEM" == "MINGW32" ]; then #Launch MinGW32 for this
  libs="${libs}/x86" #Path for 32 bit Lua libraries
  suffix="32"
else #Launch anything else for this
  libs="${libs}/x64" #Path for 64 bit Lua libraries
  suffix="64"
fi

echo "BaseDir:  [$basedir]"
echo "Name:     [$name]"
echo "BaseFile: [$file]"
echo "Libs:     [$libs]"
# exit 0


gcc -O2 -c -o "${file}.o" "${file}.c"
gcc -O -shared -o "${file}${suffix}.dll" "${file}.o" -L"$libs" -llua53
rm "${file}.o"
