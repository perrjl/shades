#!/usr/bin/env bash
wine glslang.exe -V --target-env spirv1.0 vertex.glsl -S vert -o vertex.spv
wine glslang.exe -V --target-env spirv1.0 fragment.glsl -S frag -o fragment.spv
# local Linux install of zig, your wine install may may not have required dependencies for bundled zig.exe (mingw?)
zig cc main.c -L. -lSDL3 -Iinclude -o shader.exe