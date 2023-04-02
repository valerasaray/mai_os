cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(main LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "-lstdc++ -Wl,--no-as-needed -ldl")

message("Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message("Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")



string(TIMESTAMP BUILD_TIME)
message(${BUILD_TIME})


set(COMP_ID ${CMAKE_CXX_COMPILER_ID})
set(COMP_VER ${CMAKE_CXX_COMPILER_VERSION})
set(TIME_NOW ${BUILD_TIME})

# Specify the configuration file from which the header file will be generated
configure_file (./src/config.h.in ./src/config.h @ONLY)

add_library(
        lib1 SHARED
        ./include/lib.h
        ./src/lib1.cpp
)
add_library(
        lib2 SHARED
        ./include/lib.h
        ./src/lib2.cpp
)

add_executable(main1 ./src/main1.cpp)
target_include_directories(main1 PRIVATE ./include)
target_link_libraries(main1 PRIVATE lib1 dl)

add_executable(main2 ./src/main1.cpp)
target_include_directories(main2 PRIVATE ./include)
target_link_libraries(main2 PRIVATE lib2 dl)

add_executable(main ./src/main2.cpp)
target_include_directories(main PRIVATE ./include)
