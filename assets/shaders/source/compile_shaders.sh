#/usr/bin/bash

glslangValidator -V assets/shaders/source/tile.frag -o assets/shaders/tile.frag.spv
glslangValidator -V assets/shaders/source/tile.vert -o assets/shaders/tile.vert.spv
glslangValidator -V assets/shaders/source/ui.vert -o assets/shaders/ui.vert.spv