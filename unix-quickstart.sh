#/usr/bin/bash

git clone https://github.com/lucas-kuchel/engine && cd engine && mkdir build && cmake --preset debug -B build && cmake --build build && ./build/engine