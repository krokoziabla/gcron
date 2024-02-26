lib = library(
    'gcron',
    'annual.c',
    'cron_source.c',
    'gcron.c',
    include_directories: inc,
    dependencies: [glib],
    c_args: ['-Wall','-Wextra'],
    install: true)

test(
    'test',
    executable(
      'run',
      'test/main.c',
      'test/prepare.c',
      'test/schedule.c',
      'test/annual.c',
      include_directories: inc,
      link_with: lib,
      dependencies: [glib]))

