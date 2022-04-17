project('gcron', 'c', version: '0.0')

inc = include_directories('include')

lib = library(
    meson.project_name(),
    'src/cron_source.c',
    'src/gcron.c',
    dependencies: [
        dependency('glib-2.0', version: '>=2.38'),
    ],
    install: true)

install_headers('include/cron_source.h')

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
    include_directories: inc,
    link_with: lib,
    dependencies: [
        dependency('glib-2.0', version: '>=2.38'),
    ]))
