#!/bin/bash

if [ ! -e "${HOME}/patchelf/patchelf" ]; then
	wget https://nixos.org/releases/patchelf/patchelf-0.9/patchelf-0.9.tar.gz
	tar -zxvf patchelf-0.9.tar.gz
	cd patchelf-0.9
	./configure --prefix=$HOME/patchelf
	make
	make install
else
  echo 'Using cached directory.';
fi