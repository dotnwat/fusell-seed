project('fusell-seed', 'cpp',
  meson_version: '>=1.00.0',
  default_options : [
    'cpp_std=c++20',
    'warning_level=1',
  ])

fuse = dependency('fuse')

nnn_dep = declare_dependency(
  include_directories: include_directories('include', 'spdlog/include'),
  compile_args: ['-DFUSE_USE_VERSION=35'],
  dependencies: [fuse])

executable('heap_fs',
  'heap_fs.cc',
  dependencies: [nnn_dep])
