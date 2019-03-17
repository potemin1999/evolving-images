cmake_minimum_required(VERSION 3.13)
project(evolving-images)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -rdynamic -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
file(GLOB JAVALIB_SOURCES "src/support/*.cpp")
set(JAVALIB_LIBRARY javalang)
file(GLOB EVOIMG_SOURCES "src/main/*.cpp")
set(EVOIMG_EXECUTABLE evolving-images)
file(GLOB GA_SOURCES "src/ga/*.cpp")
set(GA_LIBRARY ga)

#dependencies
find_package(PNG REQUIRED)

#building
add_library(${JAVALIB_LIBRARY} STATIC ${JAVALIB_SOURCES})
target_link_libraries(${JAVALIB_LIBRARY} png)
target_link_libraries(${JAVALIB_LIBRARY} unwind)
target_link_libraries(${JAVALIB_LIBRARY} stdc++)

add_library(${GA_LIBRARY} STATIC ${GA_SOURCES})
include_directories(${GA_LIBRARY} ${PROJECT_SOURCE_DIR}/src/support)
target_link_libraries(${GA_LIBRARY} ${JAVALIB_LIBRARY})

add_executable(${EVOIMG_EXECUTABLE} ${EVOIMG_SOURCES})
include_directories(${EVOIMG_EXECUTABLE} ${PROJECT_SOURCE_DIR}/src/ga)
include_directories(${EVOIMG_EXECUTABLE} ${PROJECT_SOURCE_DIR}/src/support)
target_link_libraries(${EVOIMG_EXECUTABLE} ${GA_LIBRARY})
target_link_libraries(${EVOIMG_EXECUTABLE} ${JAVALIB_LIBRARY})