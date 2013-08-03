# About

Fork of http://fallabs.com/tokyopromenade/ 0.9.25

# Quickstart

```sh
./configure --prefix=$HOME/.local --enable-lua --enable-fcgi --with-tc=$HOME/.local --with-lua=/usr
make
make install

#deployment
mkdir ~/Documents/www
cd ~/Documents/www
cp ~/.local/libexec/promenade.* .
cp ~/.local/share/tokyopromenade/promenade.* .
cp ~/.local/share/tokyopromenade/passwd.txt . 
prommgr create promenade.tct
prommgr import promenade.tct ~/.local/share/tokyopromenade/misc/help-*
mkdir upload
```

