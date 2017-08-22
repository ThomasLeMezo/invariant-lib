#!/bin/bash

if [ ! -e "${HOME}/patchelf-0.9/build/patchelf" ]; then
	wget https://nixos.org/releases/patchelf/patchelf-0.9/patchelf-0.9.tar.gz
	tar -zxvf patchelf-0.9.tar.gz
	cd patchelf-0.9
	./configure
	make
	mkdir build
	cp src/patchelf build/
else
  echo 'Using cached directory.';
fi