# Graphics Library 
A 2D and 3D graphics library with vulkan or opengl backend.
See `examples/` folder for simple programs using this library.
More complete programs are listed below.

## Features:

Graphics Features:
* Import and Draw 3D models, or generate your own (limited support for materials)
* Import and Draw image textures, or generate your own
* Import and Draw fonts
* 3D Skeletal Animation
* Optional Resource Pools - keep some assets loaded, load and unload other assets

Non-Graphics Features:
* Simple keyboard/mouse/controller input querying
* First and Third person camera classes
* audio file loading and playing

## Projects using this framework:

All made by me for various game jams

* [Orbbit](https://noamzeise.com/gamejam/2024/02/27/Orbbit.html) - 2024 - 2D/3D planet gravity game
* [Cat Flat](https://noamzeise.com/gamejam/2024/01/29/CatFlat.html) - 2024 - 2D physics based fish frying game
* [Meditative Marble](https://noamzeise.com/gamejam/2023/12/16/Meditative-Marble.html) - 2023 - 3D physics and procedurally loading world 
* [Space Flight Explorer](https://noamzeise.com/gamejam/2023/09/25/Space-Flight.html) - 2023 - 3D Space Sim, GameBoy Graphics and Controls
* [Robyn Hood](https://noamzeise.com/gamejam/2022/09/28/Robyn-Hood.html) - 2022 - 2D On-rails Stealth Game
* [Trials of The Pharaoh](https://noamzeise.com/gamejam/2022/05/30/TrialsOfThePharaoh.html) - 2022 - 2D Light Ray Puzzle Game

# Setup

Cmake is required for building on all platforms.

### linux with apt on debian based systems

vulkan tools / vulkan loader / validation layers / spriv compilers
```
$ sudo apt-get install vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools
```
test vulkan works
```
$ vkcube
```
audio library dependancies
```
$ sudo apt-get install libsndfile1-dev libasound-dev portaudio19-dev
```

### windows

* Download the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/), for getting the vulkan validation layers and the tools for compiling shaders into spirv.

* download a [libsndfile](https://github.com/libsndfile/libsndfile/releases) build 
(self-building is annoying as there are many dependancies). Extract the zip and note the path.

* download [portaudio](http://files.portaudio.com/docs/v19-doxydocs/compile_windows.html) code, compile and note the build path.

You will need to specify `sndfile_DIR` and `portaudio_DIR` with the cmake folders of those 
libraries, so in the next part instead of `cmake ..` do
```
cmake .. -D portaudio_DIR="path_to_portaudio_build/cmake/portaudio" -D sndfile_DIR="path_to_sndfile_build/cmake/"
```

# Build with example using CMake

```
git clone --recurse-submodules https://github.com/NoamZeise/graphics-env.git
cd graphics-env
mkdir build && cd build
cmake ..
cmake --build .
```
Then example binaries should be in `examples/` within the build folder . 
Note that _PortAudio_ and _Libsndfile_ wont be built with this, 
so on windows you'll need to copy the dlls for these libraries 
from their build folders to beside the binaries.


# Customizing the build

## Cmake Flags

* NO\_AUDIO -> don't use sndfile or portaudio. Audio functions will not work.
* NO\_FREETYPE -> don't use Freetype. Font file loading will not work.
* NO\_ASSIMP -> don't use Assimp. Loading 3D model files won't work. 
* NO\_VULKAN -> don't use Vulkan backend.
* NO\_OPENGL -> don't use OpenGL backend
* GRAPHICS\_STATIC -> build libraries statically

## Enabling other 3D model formats

To use formats other than those enabled by default, you must set `ASSIMP_BUILD_XYZ_IMPORTER` 
to true, where `XYZ` is your format, before loading the cmake files for this project.

For example to enable the blend format, you would have `set(ASSIMP_BUILD_BLEND_IMPORTER TRUE)`
somewhere in your cmake file before calling `add_subdirectory(graphics-env)`. 
Check the [assimp](https://assimp.org/) docs for more info 
on supported formats.

You should then be able to load these newly enabled formats the same as you load the default ones.

# Common Build Errors

##### Missing <vulkan/...> error
pass `-D VULKAN_HEADERS_INSTALL_DIR=/your/path/to/your/installed/vulkan/headers`
to cmake when generating this project

##### Missing sndfile or portaudio dirs
try building anyways, as they may be picked up at buildtime (especially with mingw or linux). 
If there are still issues (usually for windows with MSVC compiler),
assuming you have downloaded release builds of `sndfile` and `portaudio`,
you can pass to cmake these options.
- `-D sndfile_DIR=sndfile-build-folder/cmake`
- `-D portaudio_DIR=portaudio-build-folder/cmake/portaudio`

If you don't need audio or want to use your own audio lib you can pass
`-D NO_AUDIO=true` to disable these dependancies.

# Notes

### Proper export settings for 3D modelling software

When exporting your models, ensure you are using relative paths to textures.
Also ensure that models are exported with the Z Up direction option.

### External libraries and their uses:

key:
- included - this project is a submodule or included in the repo
- OPTIONAL - this library can be toggled with a cmake flag
- external - requires downloading external libraries

* included: [volk](https://github.com/zeux/volk) dynamically loads pointers to vulkan from driver
* included: [GLFW](https://www.glfw.org/) handles windowing and input
* included: [GLM](https://github.com/g-truc/glm) handles glsl datatypes and linear algebra
* included: [stb_image.h](https://github.com/nothings/stb) handles image loading
* included OPTIONAL:   [Assimp](https://github.com/assimp/assimp) handles model loading
* included OPTIONAL:   [freetype2](https://freetype.org/) handles font loading
* external OPTIONAL:   [Vulkan SDK](https://vulkan.lunarg.com/) for vulkan type definitions, used by volk
* external OPTIONAL:   [libsndfile](https://github.com/libsndfile/libsndfile) handles audio file loading
* external OPTIONAL:   [portaudio](http://www.portaudio.com/) handles cross platform audio playback


# Todo list:
bugs:
* instability reported on some AMD GPUs (unable to test this directly)

features:
* user created fonts
* define new shader pipelines outside of render
* better model material support (right now just diffuse colour)
