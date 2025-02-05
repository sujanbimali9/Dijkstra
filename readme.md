# Dijkstra's Algorithm

## Prerequisites

- C++ Compiler (GCC, Clang, or MSVC)
- CMake
- vcpkg (for dependency management)

## Installing vcpkg

```sh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

./bootstrap-vcpkg.sh  # Linux/macOS
./bootstrap-vcpkg.bat  # Windows
```

## Setting Up vcpkg

### Set Up vcpkg in CMake

Update the `CMakeUserPresets.json` file to set the vcpkg root path:

```json
{
  "version": 2,
  "configurePresets": [
    {
      "name": "default",
      "inherits": "vcpkg",
      "environment": {
        "VCPKG_ROOT": "path-to-vcpkg-root" // update it to root of vcpkg
      }
    }
  ]
}
```

### Update CMake Prefix Path

Modify `CMakeLists.txt` to include the vcpkg installation path:

```cmake
set(CMAKE_PREFIX_PATH "path-to-vcpkg/installed/x64-linux/share" ${CMAKE_PREFIX_PATH})
```

Replace `x64-linux` with your system's triplet (e.g., `x64-windows`, `x64-macos`).

## Building the Project

Run the following commands:

```sh
cmake --preset=default #first build will take some time to download and build required library
cmake --build build
```

## Running the Program

```sh
./build/DIJKSTRA
```
