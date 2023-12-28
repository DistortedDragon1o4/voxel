# Voxel

![GitHub license](https://img.shields.io/github/license/DistortedDragon1o4/voxel.svg)
![GitHub issues](https://img.shields.io/github/issues/DistortedDragon1o4/voxel.svg)
![GitHub stars](https://img.shields.io/github/stars/DistortedDragon1o4/voxel)

This is an experimental voxel engine written by in C++

It literally has no purpose to exist but it does. If you want to try it out do so at your own risk.

## Features:
- It works (i hope)
- The world is infinite
- Has a sort of block format, which enables complex block models, but there is no way to import models so good luck writing that in code
- A very weird and barebones implementation of a day/night cycle shader
- Block breaking and placing
- Also has a cool HUD powered by ImGUI
- Optimisations like frustum culling and "occlusion culling" (but the time taken to generate and compute visibility of chunks has increased)

## Stuff not there but will be there soon(ish):
- A proper json file format to define blocks, so you can have your own blocks
- A lighting engine
- A proper modular way to generate chunks

## Screenshots:
![Screenshot](https://raw.githubusercontent.com/DistortedDragon1o4/voxel/main/screenshots/0.png)
![Screenshot](https://raw.githubusercontent.com/DistortedDragon1o4/voxel/main/screenshots/1.png)
![Screenshot](https://raw.githubusercontent.com/DistortedDragon1o4/voxel/main/screenshots/3.png)

## Special Thanks:
- Ofcourse the Khoros group for OpenGL and all
- [FastNoise2](https://github.com/Auburn/FastNoise2) by Auburn for generating chunks
- [stb](https://github.com/nothings/stb) by nothings for giving a way to import images
- [Dear ImGui](https://github.com/ocornut/imgui) by ocornut for helping me do the thing i was the most nervous about, GUIs
- Minecraft for the inspiration
- [Sodium](https://github.com/CaffeineMC/sodium-fabric) by CaffeineMC for inspiring me to optimise my voxel engine like crazy

## Running:
First of all, don't
I would just plain discourage you from running this project unless you know how stuff works and have the time and effort to fix the weird errors that pop up

Also I have no idea if this will compile, let alone run on any non Linux systems as I developed it on Linux and do not have any other OS to test on

### On Linux:
1. First clone the project
2. Open a terminal in project root and run `xmake` this will install the dependencies and try to compile the project, if it compiles, you may skip to step 5
3. If it does not compile, just report it on the Issues page
4. If you are on any other architechture other than x86_64, you will need to download the files of [FastNoise2](https://github.com/Auburn/FastNoise2/releases/tag/v0.10.0-alpha) for your respective architecture and put the files of include and lib in the correct places
5. To run the programme `LD_LIBRARY_PATH=[path to the lib directory in project] ./build/linux/[your architecture]/release/voxel [absolute path to project root]`

## Reporting Bugs:
I know that there are a lot of bugs, if you still feel the urge to report a bug (or suggest a fix maybe) please ensure the following while you report it

- Describe the issue properly
- Write how it may be reproduced, if it can
- Include images/videos if you can
- Mention what may be causing it if you can
- Do not report random segmentation faults without any context

Remember that not all issues may be straight up crashes or errors, the issue may just be some rendering problem, so yeah properly report them if you can

Have a nice day!! 😜
