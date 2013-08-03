#!/bin/bash

WITH_TC="${1:-$HOME/.local}"
WITH_LUA="${2:-/usr}"
BUILD_DIR="$PWD/build"
WWW_DIR="$PWD/www"

./configure --prefix='' --enable-lua --with-tc="$WITH_TC" --with-lua="$WITH_LUA"
make -j4
make DESTDIR="$BUILD_DIR" install

mkdir -p "$WWW_DIR/upload"
cp "$BUILD_DIR/libexec/promenade."*                 "$WWW_DIR" 
cp "$BUILD_DIR/share/tokyopromenade/promenade."*    "$WWW_DIR" 
cp "$BUILD_DIR/share/tokyopromenade/passwd.txt"     "$WWW_DIR" 

prommgr create "$WWW_DIR/promenade.tct"
prommgr import "$WWW_DIR/promenade.tct" "$BUILD_DIR/share/tokyopromenade/misc/help-"*

cp thttpd.conf thttpd.start.bash thttpd.stop.bash "$WWW_DIR"

echo "Your site ready at $WWW_DIR"
echo "  cd $WWW_DIR"
echo "  ./thttpd.start.bash"
echo "  And go to http://localhost:8080/promenade.cgi"
