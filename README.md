# ELTE Video Assistant Referee

## Project structure

```
.
├── CMakeLists.txt
├── CMakeSettings.json                  # CMake configuration for Visual Studio
├── LICENSE
├── README.md
├── cmake                               # Generic reusable cmake files
│   ├── compilerWarnings.cmake          # Compiler warnings
│   ├── conanConfig.cmake               # Conan configuration
│   └── settings.cmake                  # General CMake settings and code analysis
├── conanfile.txt                       # 3PP dependencies to install with Conan
├── data                                # Test images
│   ├── DSCF0137.jpeg
│   ├── DSCF0142.jpeg
│   ├── M4Sport_screenshot3.png
│   └── M4Sport_screenshot5.png
├── res                                 # Resources for the application
│   ├── bindings                        # ImGui bindings for different APIs
│   │   ├── imgui_impl_glfw.cpp
│   │   ├── imgui_impl_glfw.h
│   │   ├── imgui_impl_opengl2.cpp
│   │   └── imgui_impl_opengl2.h
│   └── pitch.png                       # Reference image of a football pitch
├── src
│   ├── CMakeLists.txt
│   ├── app.h                           # GUI implementation / View
│   ├── gui.cc
│   ├── gui.h                           # Abstraction around the 3PP GUI library
│   ├── logging.h                       # Abstraction around the 3PP logging library
│   ├── main.cc                         # Entrypoint
│   ├── model.h                         # Business logic
│   └── types.h                         # Internally used types
└── tools                               # Tools and scripts for the package
    └── build                           # Python script for enhancing the build experience on Unix-like systems
```

## Architecture

The ELTE Video Assistant Referee application is separated into two main parts,
the user interface and the business logic.

## Algorithms

### Homography

```
Mat cv::findHomography(InputArray srcPoints,
                       InputArray dstPoints,
                       int method = 0,
                       double ransacReprojThreshold = 3,
                       OutputArray mask = noArray(),
                       const int maxIters = 2000,
                       const double confidence = 0.995);
```

The function [cv::findHomography](https://docs.opencv.org/4.x/d9/d0c/group__calib3d.html#ga4abc2ece9fab9398f2e560d53c8c9780)
finds and returns the perspective transformation $H$ between the source and the destination planes:

$$ s_i \begin{bmatrix} x'_i \\ y'_i \\ 1 \end{bmatrix} \sim H \begin{bmatrix} x_i \\ y_i \\ 1 \end{bmatrix} $$

so that the back-projection error

$$ \sum_i \left( x' - \frac{h_{11} x_i + h_{12} y_i + h_{13}}{h_{31} x_i + h_{32} y_i + h_{33}} \right)^2 + \left( y' - \frac{h_{21} x_i + h_{22} y_i + h_{23}}{h_{31} x_i + h_{32} y_i + h_{33}} \right)^2 $$

$$ \sum_i \left( x'_i - \frac{h_{11} x_i + h_{12} y_i + h_{13}}{h_{31} x_i + h_{32} y_i + h_{33}} \right)^2 + \left( y'_i- \frac{h_{21} x_i + h_{22} y_i + h_{23}}{h_{31} x_i + h_{32} y_i + h_{33}} \right)^2 $$

is minimized.

The function is used to find initial intrinsic and extrinsic matrices. Homography matrix is determined
up to a scale. Thus, it is normalized so that $h_{33} = 1$. Note that whenever an $H$ matrix cannot be
estimated, an empty one will be returned.

### Perspective transformation

```
void cv::perspectiveTransform(InputArray src, OutputArray dst, InputArray m);
```

The function [cv::perspectiveTransform](https://docs.opencv.org/3.4/d2/de8/group__core__array.html#gad327659ac03e5fd6894b90025e6900a7)
transforms every element of `src` by treating it as a 2D or 3D vector, in the following way:

$$ (x, y, z) -> \left( \frac{x'}{w}, \frac{y'}{w}, \frac{z'}{w} \right) $$


where

$$ (x', y', z', w') = \text{mat} * \begin{bmatrix} x & y & z & 1 \end{bmatrix} $$

and

$$ w = \begin{cases} w' & \text{if }w' \neq 0 \\ \inf & \text{otherwise} \end{cases} $$

Here a 3D vector transformation is shown. In case of a 2D vector transformation, the $z$ component is omitted.

## Business logic

## Graphical User Interface

[Dear ImGui](https://github.com/ocornut/imgui) is a bloat-free graphical user interface library for C++.
It is designed to enable fast iterations and to empower programmers to create content creation tools and
visualization / debug tools. Dear ImGui is one possible implementation of an idea generally described
as the IMGUI (Immediate Mode GUI) paradigm.

## Build script

Convenience build script for improved developer workflow.

Everything beyond double dash (`--`) is forwarded as is to CMake configuring.

The script has the ability to run a given target (optionally in GDB) with
the command line given arguments.

It manages separate build directories for different build configurations.

`compile_commands.json` is automatically linked to the root of the project.
