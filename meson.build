project(
  'GCron', 'c',
  version: '0.0',
  default_options: ['c_std=c17'])

inc = include_directories('include')
glib = dependency('glib-2.0', version: '>=2.44')

install_headers('include/gcron.h')

subdir('src')
subdir('example')

gnome = import('gnome')

gir = gnome.generate_gir(
    lib,
    header: 'gcron.h',
    includes: ['GLib-2.0'],
    sources: ['include/gcron.h'],
    namespace: meson.project_name(),
    nsversion: '0',
    install: true)

gnome.generate_vapi(
    '@0@-@1@'.format(meson.project_name(), meson.project_version()),
    sources: gir[0],
    install: true)

import('pkgconfig').generate(lib)
