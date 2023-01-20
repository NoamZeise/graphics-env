# OpenGL-Environment
2D/3D Renderer - same functions as [vulkan-environment](https://github.com/NoamZeise/Vulkan-Environment), but with OpenGL backend. See that repo for more info.



# setup

### windows

* put glad opengl include into your includes: https://glad.dav1d.de/ pick an opengl version at or above 3.3

* download [glfw3](https://www.glfw.org/), compile and put in your lib and include directories

* download [glm](https://github.com/g-truc/glm), it is header only so put the headers in your include directory

* download [freetype](https://freetype.org/download.html) compile and put in your lib and include directories

* download [assimp](https://github.com/assimp/assimp/blob/master/Build.md) compile and put in your lib and include directories, and distribute the dll with your binaries

* download [libsndfile](http://www.mega-nerd.com/libsndfile/#Download) compile and put in your lib and include directories, and distrubute dll with your binaries

* download [portaudio](http://files.portaudio.com/docs/v19-doxydocs/compile_windows.html) compile and put in your lib and include directories, and distrubute dll with your binaries

* set your lib and include paths at the start of the cmake file
```
#windows only
set(Lib "Path/to/lib")
set(Include "Path/to/include")
```

### linux with apt
put glad opengl include into your includes: https://glad.dav1d.de/ pick an opengl version at or above 3.3

install libraries
```
$ sudo apt-get install libglfw3-dev libglm-dev libfreetype-dev libassimp-dev libsndfile1-dev libasound-dev portaudio19-dev
```
