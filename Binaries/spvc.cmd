@echo off
glslangValidator.exe -V ../Engine/Source/Shader/simple.vert -o ../Engine/Source/Shader/simple.vert.spv
glslangValidator.exe -V ../Engine/Source/Shader/simple.frag -o ../Engine/Source/Shader/simple.frag.spv