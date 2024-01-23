@echo off
glslangValidator.exe -V ../Engine/Shaders/simple.vert -o ../Engine/Shaders/simple.vert.spv
glslangValidator.exe -V ../Engine/Shaders/simple.frag -o ../Engine/Shaders/simple.frag.spv