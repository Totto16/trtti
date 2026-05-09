project(
    'trtti',
    'c',
    license: 'MIT',
    meson_version: '>=1.3.0',
    default_options: {
        'buildtype': 'release',
        'optimization': '3',
        'c_std': ['c23', 'c11'],
        'warning_level': '3',
        'werror': 'true',
        'b_ndebug': 'if-release',
    },
    version: '1.0.1',
)

inc_dirs = []

subdir('src')

trtti_dep = declare_dependency(
    include_directories: inc_dirs,
    version: meson.project_version(),
)

meson.override_dependency('trtti', trtti_dep)


