cmake_minimum_required(VERSION 3.13)
project(evolving-images)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_EXE_LINKER_FLAGS "-rdynamic -lpng -lstdc++")

set(evolving-images-mainsrc
        src/Bitmap.cpp
        src/Exception.cpp
        src/Main.cpp
        )

add_executable(evolving-images ${evolving-images-mainsrc})