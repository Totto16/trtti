src_files += files(
    'json.c',
    'json.h',
    '_impl/utils.h',
    '_impl/utf8.h',
    '_impl/utf8.c',
    '_impl/path.h',
    '_impl/path.c',
)


install_headers('json.h', preserve_path : true)

c_variant_generator = find_program(
    'c_variant_generator',
    version: ['>=1.0.0', '<2'],
)

json_variants_generated_tgt = custom_target(
    'json_variants',
    command: [
        c_variant_generator,
        '-o',
        '@OUTPUT@',
        '-i',
        '@INPUT@',
    ],
    input: [meson.project_source_root() / 'src' / 'json.variants.def.json'],
    output: ['json_variants.h'],
    build_subdir: 'generated' / 'json' / 'variants',
    install: true,
    install_dir: 'generated'
)

json_variants_dep = declare_dependency(
    version: meson.project_version(),
    sources: json_variants_generated_tgt,
)


cc = meson.get_compiler('c')

stdc_version = cc.get_define('__STDC_VERSION__ ')
stdc_version_int = stdc_version.replace('L', '').to_int()
if stdc_version_int >= 202000
    compile_flags += '-D_TJSON_COMPILE_WITH_NARROWED_ENUMS'
endif


