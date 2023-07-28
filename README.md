# Work in progress Game Framework
A 2D and 3D renderer with vulkan or opengl backend, add rendering and update code into app.cpp. Works with Windows or Linux.

## Features:

* Simultaneous 2D and 3D rendering
* Import and Draw 3D models, or import/generate your own (limited support for materials)
* Import and Draw image textures 
* Import and Draw fonts
* Play .wav and .ogg audio files
* 3D Skeletal Animation

## Projects using this framework:
* [Robyn Hood](https://github.com/NoamZeise/Robyn-Hood) - 2022 - 2D On-rails Stealth Game
* [Trials of The Pharaoh](https://github.com/NoamZeise/TrailsOfThePharaoh) - 2022 - 2D Light Ray Puzzle Game

## External libraries and their uses:

* included: [volk](https://github.com/zeux/volk) dynamically loads pointers to vulkan from driver
* included: [GLFW](https://www.glfw.org/) handles windowing and input
* included: [GLM](https://github.com/g-truc/glm) handles glsl datatypes and linear algebra
* included: [stb_image.h](https://github.com/nothings/stb) handles image loading
* included OPTIONAL:   [Assimp](https://github.com/assimp/assimp) handles model loading
* included OPTIONAL:   [freetype2](https://freetype.org/) handles font loading
* external:   [Vulkan SDK](https://vulkan.lunarg.com/) for vulkan type definitions, used by volk
* external:   [libsndfile](https://github.com/libsndfile/libsndfile) handles audio file loading
* external:   [portaudio](http://www.portaudio.com/) handles cross platform audio playback

# setup

Cmake is required for building on all platforms.

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

* download [libsndfile](http://www.mega-nerd.com/libsndfile/#Download) compile and put in your lib and include directories, and distrubute dll with your binaries

* download [portaudio](http://files.portaudio.com/docs/v19-doxydocs/compile_windows.html) compile and put in your lib and include directories, and distrubute dll with your binaries

You may need to specify `sndfile_DIR` and `portaudio_DIR` with the cmake folders of those libraries ( ie where the `portaudioConfig.cmake` files are).

Download the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/), for getting the vulkan validation layers and the tools for compiling shaders into spirv.

# Build with example using CMake

```
git clone --recurse-submodules https://github.com/NoamZeise/Graphics-Environment.git
cd Graphics-Environment
mkdir build && cd build
cmake .. -DGRAPHICS_BUILD_EXAMPLE=true
cmake --build .
```
Then the built binary should be in /build/examples/. Note that PortAudio and Libsndfile dlls wont be built with this, so on windows you'll need to copy the dlls for those into the same path as the example binary.

# Enabling other 3D model formats

To use formats other than `.obj` and `.fbx`, you must set `ASSIMP_BUILD_XYZ_IMPORTER` to true, where `XYZ` is your format, before loading the cmake files for this project.

For example to enable the gltf format, you would have `set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)` somewhere in your cmake file before calling `add_subdirectory(Graphics-Environment)`. Check the [assimp](https://assimp.org/) docs for more info on supported formats.

You should then be able to load these newly enabled formats the same as you load the default ones.

# Todo list:
bugs:
* instability reported on some AMD GPUs (unable to test this directly)

features:
* make multiple render passes optional
* define new shader pipelines outside of render
* unload old and load new textures while using renderer
