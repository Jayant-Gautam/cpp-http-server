# HTTP Server

A simple HTTP server implemented in C++ using raw sockets.


## Prerequisites

- CMake (version 3.16 or higher)
- C++17 compatible compiler (e.g., g++)

## Building the Project

1. Clone or navigate to the project directory.
2. Create a build directory:
   ```
   mkdir build
   cd build
   ```
3. Generate build files with CMake:
   ```
   cmake ..
   ```
4. Build the project:
   ```
   make
   ```

## Running the Server

After building, run the server executable:

```
./server
```

## Project Structure

- `src/main.cpp`: Entry point of the application.
- `src/server.cpp`: Implementation of the Server class.
- `include/server.hpp`: Header file for the Server class.
- `CMakeLists.txt`: CMake build configuration.

## Notes

This is a basic implementation for educational purposes. It does not handle multiple concurrent connections efficiently and lacks advanced HTTP features like routing, headers parsing, etc.