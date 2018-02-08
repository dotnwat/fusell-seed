cmake_minimum_required(VERSION 2.8.12)

project(fuse-boilerplate)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# require c++11 (Thanks Ceph)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if (COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -g")

find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE REQUIRED fuse)

add_executable(main main.cc inode.cc filesystem.cc inode_index.cc)
target_include_directories(main PRIVATE ${FUSE_INCLUDE_DIRS})
target_link_libraries(main ${FUSE_LIBRARIES})
target_compile_options(main PRIVATE ${FUSE_CFLAGS})
target_compile_definitions(main PRIVATE FUSE_USE_VERSION=30)
