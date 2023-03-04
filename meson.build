project('fusell-seed', 'cpp',
  meson_version: '>=1.00.0',
  default_options : [
    'cpp_std=c++20',
    'warning_level=1',
  ])

fuse = dependency('fuse')

inc_dirs = [include_directories('spdlog/include'),
  include_directories('include')]

nnn_dep = declare_dependency(
  include_directories: inc_dirs,
  compile_args: ['-DFUSE_USE_VERSION=35'],
  dependencies: [fuse])

if not meson.is_subproject()
  subdir('examples')
endif
