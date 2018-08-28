cmake_minimum_required(VERSION 2.8.12)

project(fusell-seed)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# require c++17 (Thanks Ceph)
if(CMAKE_VERSION VERSION_LESS "3.8")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
  if(NOT COMPILER_SUPPORTS_CXX17)
    message(FATAL_ERROR
      "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support.")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  include(CheckCCompilerFlag)
  CHECK_C_COMPILER_FLAG("-std=gnu99" COMPILER_SUPPORTS_GNU99)
  if(NOT COMPILER_SUPPORTS_GNU99)
    message(FATAL_ERROR
      "The compiler ${CMAKE_C_COMPILER} has no GNU C99 support.")
  endif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_C_STANDARD 99)
  # we use `asm()` to inline assembly, so enable the GNU extension
  set(CMAKE_C_EXTENSIONS ON)
  set(C_STANDARD_REQUIRED ON)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -g")

find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE REQUIRED fuse)

include_directories(spdlog/include)

add_executable(main main.cc inode.cc filesystem.cc inode_index.cc)
target_include_directories(main PRIVATE ${FUSE_INCLUDE_DIRS})
target_link_libraries(main ${FUSE_LIBRARIES})
target_compile_options(main PRIVATE ${FUSE_CFLAGS})
target_compile_definitions(main PRIVATE FUSE_USE_VERSION=30)
