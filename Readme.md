## How to build

Don't.

Requirements:
- [CommonLibF4](https://github.com/libxse/commonlibf4/tree/frozen-1.10.63) with branch frozen-1.10.63
- Visual Studio 2022
- Visual Studio Code with CMake extension from Microsoft

1. Adjust CMakeLists.txt to specify location of the Commonlibf4/Commonlibf4 folder.
2. Open this repo in VS Code
3. Ctrl + Shift + P -> CMake: Select Kit -> anything x64
4. Ctrl + Shift + P -> CMake: Configure
5. Ctrl + Shift + P -> CMake: Build
6. Cry it doesn't work, fix commonlibf4 files: memory.h, Actor.h, FormComponents.h
7. Ctrl + Shift + P -> CMake: Build