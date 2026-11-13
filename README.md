# Shades
Self-contained demo for quickly running shaders on the desktop without model/vertex data.
Zig is used as a C compiler, glslang for GLSL -> SPIRV generation, and SDL3 for windowing and its GPU API.
Note the Zig executable is stored using Git LFS, so bring your own compiler or be sure to fetch it through the relevant
Git LFS command.
This demo is targeted for Windows, please see shader.bat and build.bat for how you would build this for other platforms
with your local installs of a C compiler, glslang, and SDL3.