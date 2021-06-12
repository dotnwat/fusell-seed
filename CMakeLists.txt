cmake_minimum_required(VERSION 3.15)

project(fusell-seed)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -g")

if(NOT CMAKE_BUILD_TYPE MATCHES Release)
  # non-release: default, debug, etc...
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -fsanitize=undefined -fsanitize=address")
endif()

find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE REQUIRED fuse)

include_directories(spdlog/include)

add_executable(heap_fs heap_fs.cc)
target_include_directories(heap_fs PRIVATE ${FUSE_INCLUDE_DIRS})
target_link_libraries(heap_fs ${FUSE_LIBRARIES})
target_compile_options(heap_fs PRIVATE ${FUSE_CFLAGS})
target_compile_definitions(heap_fs PRIVATE FUSE_USE_VERSION=30)

enable_testing()
add_subdirectory(test)
