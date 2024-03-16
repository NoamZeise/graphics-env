# Work in progress graphics library
A 2D and 3D renderer with vulkan or opengl backend.
See `examples/` folder for simple programs using this library.
More complete programs are listed below.

## Features:

Graphics Features
* Import and Draw 3D models, or generate your own (limited support for materials)
* Import and Draw image textures, or generate your own
* Import and Draw fonts
* 3D Skeletal Animation
* Optional Resource Pools - keep some assets loaded, load and unload other assets
Non-Graphics Features
* Simple keyboard/mouse/controller input 
* First and Third person cameras
* audio playback

## Projects using this framework:
* [Orbbit](https://github.com/NoamZeise/dsg-2024) - 2024 - 2D/3D planet gravity game
* [Cat Flat](https://github.com/NoamZeise/GGJ2024) - 2024 - 2D physics based fish frying game
* [Meditative Marble](https://github.com/NoamZeise/MeditativeMarble) - 2023 - 3D physics and procedurally loading world 
* [Space Flight Explorer](https://github.com/NoamZeise/gbjam11) - 2023 - 3D Space Sim, GameBoy Graphics and Controls
* [Robyn Hood](https://github.com/NoamZeise/Robyn-Hood) - 2022 - 2D On-rails Stealth Game
* [Trials of The Pharaoh](https://github.com/NoamZeise/TrailsOfThePharaoh) - 2022 - 2D Light Ray Puzzle Game

## External libraries and their uses:

included - this project is a submodule or included in the repo
OPTIONAL - this library can be toggled with a cmake flag
external - requires downloading external libraries

* included: [volk](https://github.com/zeux/volk) dynamically loads pointers to vulkan from driver
* included: [GLFW](https://www.glfw.org/) handles windowing and input
* included: [GLM](https://github.com/g-truc/glm) handles glsl datatypes and linear algebra
* included: [stb_image.h](https://github.com/nothings/stb) handles image loading
* included OPTIONAL:   [Assimp](https://github.com/assimp/assimp) handles model loading
* included OPTIONAL:   [freetype2](https://freetype.org/) handles font loading
* external OPTIONAL:   [Vulkan SDK](https://vulkan.lunarg.com/) for vulkan type definitions, used by volk
* external OPTIONAL:   [libsndfile](https://github.com/libsndfile/libsndfile) handles audio file loading
* external OPTIONAL:   [portaudio](http://www.portaudio.com/) handles cross platform audio playback

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
git clone --recurse-submodules https://github.com/NoamZeise/graphics-env.git
cd graphics-env
mkdir build && cd build
cmake ..
cmake --build .
```
Then example binaries should be in `examples/` within the build folder . Note that _PortAudio_ and _Libsndfile_ dlls wont be built with this, so on windows you'll need to copy the dlls for those into the same path as the example binaries.

## Cmake Flags

* NO\_AUDIO -> don't link to _sndfile_ or _portaudio_. Audio functions will not work.
* NO\_FREETYPE -> don't build or link to _freetype_. Font loading/drawing will not work.
* NO\_ASSIMP -> don't build or link to _assimp_. Loading 3D models from file will not work. 
* NO\_VULKAN -> don't build or link to Vulkan backend.
* NO\_OPENGL -> don't build or link to OpenGL backend
* GRAPHICS\_BUILD\_STATIC -> build everything statically


# FAQ

### Common Build Errors

* Missing <vulkan/...> error
pass `-D VULKAN_HEADERS_INSTALL_DIR=/your/path/to/your/installed/vulkan/headers`
to cmake when generating this project

### Enabling other 3D model formats

To use formats other than those enabled by default, you must set `ASSIMP_BUILD_XYZ_IMPORTER` to true, where `XYZ` is your format, before loading the cmake files for this project.

For example to enable the blend format, you would have `set(ASSIMP_BUILD_BLEND_IMPORTER TRUE)` somewhere in your cmake file before calling `add_subdirectory(graphics-env)`. 
Check the [assimp](https://assimp.org/) docs for more info 
on supported formats.

You should then be able to load these newly enabled formats the same as you load the default ones.

### Proper export settings for 3D modelling software

When exporting your models, ensure you are using relative paths to textures.
Also ensure that models are exported with the Z Up direction option.

# Todo list:
bugs:
* instability reported on some AMD GPUs (unable to test this directly)

features:
* define new shader pipelines outside of render
* better model material support (right now just diffuse colour)
