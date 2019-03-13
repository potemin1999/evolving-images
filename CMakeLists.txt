cmake_minimum_required(VERSION 3.13)
project(evolving-images)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -rdynamic -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(EVOIMG_SOURCES
        src/Bitmap.cpp
        src/Exception.cpp
        src/Main.cpp
        )
set(EVOIMG_EXECUTABLE evolving-images)

#dependencies
find_package(PNG REQUIRED)

#building
add_executable(${EVOIMG_EXECUTABLE} ${EVOIMG_SOURCES})
target_link_libraries(${EVOIMG_EXECUTABLE} png)
target_link_libraries(${EVOIMG_EXECUTABLE} unwind)
target_link_libraries(${EVOIMG_EXECUTABLE} stdc++)