# Work in progress Game Framework
A 2D and 3D renderer with vulkan or opengl backend, add rendering and update code into app.cpp. Works with Windows or Linux.

## Features:

* Simultaneous 2D and 3D rendering
* Import and Draw .fbx models -> only supports base colour image textures
* Import and Draw image textures 
* Import and Draw fonts
* Play .wav and .ogg audio files

## Projects using this framework:
* [Robyn Hood](https://github.com/NoamZeise/Robyn-Hood) - 2022 - 2D On-rails Stealth Game
* [Trials of The Pharaoh](https://github.com/NoamZeise/TrailsOfThePharaoh) - 2022 - 2D Light Ray Puzzle Game

## External libraries and their uses:

* [Vulkan lunarG](https://vulkan.lunarg.com/) for vulkan function loader, debugging, validation layers, spriv compilers
* included: [volk](https://github.com/zeux/volk) dynamically loads pointers to vulkan from driver
* included: [GLFW](https://www.glfw.org/) handles windowing and input
* included: [GLM](https://github.com/g-truc/glm) handles glsl datatypes and linear algebra
* included: [stb_image.h](https://github.com/nothings/stb) handles image loading
* included OPTIONAL:   [Assimp](https://github.com/assimp/assimp) handles model loading
* external REQUIRED:   [Vulkan](https://vulkan.lunarg.com/) for vulkan type definitions, used by volk
* external OPTIONAL:   [freetype2](https://freetype.org/) handles font loading

# setup

### linux with apt on debian based systems
vulkan tools
```
$ sudo apt-get install vulkan-tools
```
vulkan loader / validation layers / spriv compilers
```
$ sudo apt-get install libvulkan-dev vulkan-validationlayers-dev spirv-tools
```
test vulkan works
```
$ vkcube
```
additional libraries
```
$ sudo apt-get install libfreetype-dev libsndfile1-dev libasound-dev portaudio19-dev
```

### windows

* download [freetype](https://freetype.org/download.html) compile and put in your lib and include directories

* download [libsndfile](http://www.mega-nerd.com/libsndfile/#Download) compile and put in your lib and include directories, and distrubute dll with your binaries

* download [portaudio](http://files.portaudio.com/docs/v19-doxydocs/compile_windows.html) compile and put in your lib and include directories, and distrubute dll with your binaries


If using GNU compiler on window, you can use something like Msys2 and use `$ pacman -S mingw-w64-x86_64-freetype` to get freetype (similar for libsndfile and portaudio), or if not using Msys2, or using microsoft's compiler download these libraries directly and make sure cmake or your compiler can see the library files.  
You may need to specify `FREETYPE_LIBRARY` as the path to your library folder if cmake can't find it, and similarly for the other libraries

Download the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/), for getting the vulkan headers and compiling shaders into spirv. make sure the headers can be seen by your compiler.

# Todo list:
bugs:
* make first-person camera feel better

features:
* skeletal animation with ogl renderer
* make multiple render passes optional

optimisations:
* convert model data to proprietary format with another program to remove assimp dependancy from this project
* use the same pipeline layout for multiple pipelines
* unload old and load new textures while using renderer
