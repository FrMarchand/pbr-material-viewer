# PBR Material Viewer

A simple PBR/IBL material viewer with drag and drop functionality written in C++/OpenGL.

[![Real-time PBR](http://img.youtube.com/vi/5Qgu7ap8vvs/0.jpg)](http://www.youtube.com/watch?v=5Qgu7ap8vvs)

### Features

* Drag and drop an equirectangular HDR image in the viewport to automatically generate the required cubemaps for Image-Based Lighting ([hdrlabs has a nice collection of environment maps](http://www.hdrlabs.com/sibl/archive.html)).
* Drag and drop individual images in the material components (albedo, normal, metallic, roughness, ambient occlusion, displacement) to change the material's appearance ([freepbr has a nice collection of different material textures](https://freepbr.com/)).
* Adjust the displacement amount and texture scale.
* Toggle rotation and wireframe.
* Toggle and move a point-light around the scene.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. Please note that Windows is the only supported platform at the moment.

### Prerequisites

* A modern C/C++ compiler
* OpenGL 3.3+
* CMake 3.0+ installed

### Building the project

#### Git clone

```bash
 git clone https://github.com/FrMarchand/pbr-material-viewer.git
 cd pbr-material-viewer
```

#### Generate project files

```bash
 mkdir build
 cd build
 cmake ..
```

You can then open the generated project files in your IDE or build the project directly with cmake

#### Build with CMake

```
cmake --build . --config Release
```

In order to keep things simple, building this project will generate a standalone executable. Shaders and other resources are embedded in the program during compilation.

## Libraries

* [glad](https://glad.dav1d.de/) - Multi-Language Loader-Generator based on the official specs.
* [glfw](https://www.glfw.org/) - Library for creating windows, contexts and surfaces, receiving input and events.
* [glm](https://glm.g-truc.net/0.9.9/index.html) - C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications.
* [dear imgui](https://github.com/ocornut/imgui) - Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies.
* [stb-image](https://github.com/nothings/stb) - Image loading/decoding from file/memory
* [cmrc](https://github.com/vector-of-bool/cmrc) - Standalone CMake-Based C++ Resource Compiler

## Author

**François Marchand** - [FrMarchand](https://github.com/FrMarchand)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

This project was based on the PBR tutorial series provided by [learnopengl.com](https://learnopengl.com/), a great resource for anything related to OpenGL.
