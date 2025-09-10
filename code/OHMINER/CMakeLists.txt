cmake_minimum_required(VERSION 3.16.3)

# 指定默认编译器（可被命令行覆盖：-DCMAKE_C_COMPILER=... -DCMAKE_CXX_COMPILER=...）
if(NOT DEFINED CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER gcc)
endif()
if(NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER g++)
endif()

project(Hypergraph)


SET(INTEL_VTUNE "/opt/intel/oneapi/vtune/latest/")
INCLUDE_DIRECTORIES(${INTEL_VTUNE}/include)

SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/libs)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")


ADD_COMPILE_OPTIONS(-std=c++11)

find_package(OpenMP)

find_package(MPI REQUIRED)
if(MPI_FOUND)
    SET(CMAKE_CXX_COMPILER mpicxx)
endif()

add_subdirectory(src)
add_subdirectory(test)


