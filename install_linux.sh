#!/bin/sh

echo "set(PROJECT_SOURCES " > sources.cmake
find XeroPathGen -iname "*.cpp" -or -iname "*.h" >> sources.cmake
echo ")" >> sources.cmake

cmake CMakeLists.txt
make

rm sources.cmake

mkdir -p ~/.local/share/applications/
mkdir -p ~/.local/opt/xeropath2

cat xeropath2.desktop | envsubst > ~/.local/share/applications/xeropath2.desktop

cp xeropath2 ~/.local/opt/xeropath2
cp -r XeroPathGen/images ~/.local/opt/xeropath2


