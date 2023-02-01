project(
  'gcron', 'c',
  version: '0.0',
  default_options: ['c_std=c11'])

inc = include_directories('include')

lib = library(
    meson.project_name(),
    'src/annual.c',
    'src/cron_source.c',
    'src/gcron.c',
    include_directories: inc,
    dependencies: [
        meson.get_compiler('c').find_library('m', required: true),
        dependency('glib-2.0', version: '>=2.44'),
    ],
    c_args: ['-Wall','-Wextra'],
    install: true)

install_headers('include/gcron.h')

gnome = import('gnome')

gir = gnome.generate_gir(
    lib,
    namespace: 'GCron',
    nsversion: '0')

gnome.generate_vapi(
    'gcron',
    sources: gir[0],
    install: true)

import('pkgconfig').generate(lib)

test(
    'tests',
    executable(
      'tests',
      'test/main.c',
      'test/prepare.c',
      'test/schedule.c',
      include_directories: inc,
      link_with: lib,
      dependencies: [
          dependency('glib-2.0', version: '>=2.44'),
      ]))

executable(
  'example',
  'example/main.c',
  include_directories: inc,
  link_with: lib,
  dependencies: [
      dependency('glib-2.0', version: '>=2.44'),
  ])
