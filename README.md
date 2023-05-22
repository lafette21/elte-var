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

```math
\sum_i \left( x'_i - \frac{h_{11} x_i + h_{12} y_i + h_{13}}{h_{31} x_i + h_{32} y_i + h_{33}} \right)^2 + \left( y'_i - \frac{h_{21} x_i + h_{22} y_i + h_{23}}{h_{31} x_i + h_{32} y_i + h_{33}} \right)^2
```

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

### API

[Dear ImGui](https://github.com/ocornut/imgui) is a bloat-free graphical user interface library for C++.
It is designed to enable fast iterations and to empower programmers to create content creation tools and
visualization / debug tools. Dear ImGui is one possible implementation of an idea generally described
as the IMGUI (Immediate Mode GUI) paradigm.

### Menu

#### Overview

The menu bar is positioned at the top of the window. It provides users with access to various commands
and options that are organized into different menus.

The menu bar contains the following items:
- File
- Edit
- Debug

#### File Menu

The File menu provides options related to file operations.

The following items are available in the File menu:
- Load - This options enables users to load images as scenarios to check with the VAR application.
  This item triggers a dialog window for entering the path of the desired image to load.
- Save - This option allows users to save the evaluated scenarios as images to their machines.
  This item triggers a dialog window for entering the path of the desired location of the image to save to.

#### Edit Menu

The Edit menu provides options for manipulating the data loaded into the application.

The following items are available in the Edit menu:
- Generate - This option triggers specific actions within the application to evaluate the loaded scenarios.
  More on this in the [Business logic](#business-logic) section.
- Reset - This option allows users to reset their state of the scenarios.

#### Debug Menu

The Debug menu allows users to debug and troubleshoot within the development process. When triggering
this item a Debug window opens and shows different informations about the application. More on this
in the [Windows](#windows) section.

### Windows

ImGui provides different widgets and so called ui_windows to show within the main window of the application.

#### Image / Pitch

These windows have basic image viewer purposes. They are not resizeable for now because of some API limitations.
The windows are constructed in a way that clicking on the image triggers an event in which the mouse position
is saved for different purposes.

#### Image points / Pitch points

These windows are used to list the selected reference points on both images for the application to generate
the evaluation of the scenarios. This listing is done in listboxes. Selecting an item in the list allows
the user overwriting the focused item.

There are two buttons on the windows:
- Remove focus - Removes the focus from the current item in the list.
- Remove selected - Removes the selected/focused item from the list.

#### Players

This window shows the attacker and defender players position.

There are two types of buttons on the window:
- Set - Triggering this button enables the user to select a point on the Image window. This point will not be
  listed within the Image points but rather used for the players position.
- Reset - Triggering this button will reset the given player's position.

#### Debug

This window is used for displaying informations about the state of the application e.g. logic time, mouse position.

## Build script

Convenience build script for improved developer workflow.

Everything beyond double dash (`--`) is forwarded as is to CMake configuring.

The script has the ability to run a given target (optionally in GDB) with
the command line given arguments.

It manages separate build directories for different build configurations.

`compile_commands.json` is automatically linked to the root of the project.
