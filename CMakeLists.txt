cmake_minimum_required(VERSION 3.16...3.23)

# Set the project name and language
project(elte-var
    VERSION 0.0.1
    DESCRIPTION ""
    HOMEPAGE_URL "https://github.com/laFette21/elte-var"
    LANGUAGES CXX C
)

# Include cmake files
include(cmake/compilerWarnings.cmake)
include(cmake/conanConfig.cmake)
include(cmake/settings.cmake)

# Setup Conan
run_conan()

# Set project warnings
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

# Add the src directory
add_subdirectory(src)

# Copy pitch.png to build folder
configure_file(res/pitch.png ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res/pitch.png COPYONLY)

# Enable testing
enable_testing()
