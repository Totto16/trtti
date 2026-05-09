src_files += files(
    'trtti.h',
    'trtti.c',
)

include_dir = get_option('prefix') / get_option('includedir')



install_headers(
    files('trtti.h'),
    install_dir: include_dir / 'trtti',
    preserve_path: true,
)



inc_dirs += include_directories('.')


