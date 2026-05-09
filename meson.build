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
    version: '1.0.0',
)


src_files = []
inc_dirs = []

subdir('src')

link_depends = []
link_args = []

c = meson.get_compiler('c')

if c.get_linker_id() in ['ld.bfd', 'ld.lld']
    trtti_linker_script = meson.project_source_root() / 'src' / 'trtti.ld'

    link_depends += trtti_linker_script
    link_args += ['-T', trtti_linker_script]

elif c.get_linker_id() in ['ld.mold']
    error('linker ' + c.get_linker_id() + ' not supported: mold doesn\'t support linker scripts')
else
    error('linker ' + c.get_linker_id() + ' currently not supported')
endif

trtti_lib = library(
    'trtti',
    src_files,
    include_directories: inc_dirs,
    install: true,
    link_depends: [trtti_linker_script],
    link_args: ['-T', trtti_linker_script],
)

trtti_dep = declare_dependency(
    link_with: trtti_lib,
    include_directories: inc_dirs,
    version: meson.project_version(),
)

meson.override_dependency('trtti', trtti_dep)


