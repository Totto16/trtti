src_files += files(
    'tjson.c',
    'tjson.h',
    'tjson/variants.h',
    '_impl/utils.h',
    '_impl/utf8.h',
    '_impl/utf8.c',
    '_impl/path.h',
    '_impl/path.c',
)


install_headers('tjson.h', 'tjson/variants.h', preserve_path : true)

# check if we need to use our own variants for bootstrap reasons, this is implementation defined between c_variant_generator and tjson
# nothing in here should be used by users of either of the libraries, this is just a implementation detail of both libs!

tjson_fallback_to_internal_variant_dep = dependency('tjson_fallback_to_internal_variant', required: false)

if tjson_fallback_to_internal_variant_dep.found()

    if not meson.is_subproject()
        error('only allowed, when in subproject')
    endif

    global_src_root = meson.global_source_root()

    fs = import('fs')

    global_project_name = fs.name(global_src_root)

    if global_project_name != 'c_variant_generator'
        error('called from another toplevel project, this is an implementation error')
    endif

    json_variants_dep = declare_dependency(
        version: meson.project_version(),
        compile_args: ['-D_TJSON_USE_VARIANT_IMPL=0']
    )
else

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
        compile_args: ['-D_TJSON_USE_VARIANT_IMPL=1']
    )

endif






cc = meson.get_compiler('c')

stdc_version = cc.get_define('__STDC_VERSION__ ')
stdc_version_int = stdc_version.replace('L', '').to_int()
if stdc_version_int >= 202000
    compile_flags += '-D_TJSON_COMPILE_WITH_NARROWED_ENUMS'
endif

inc_dirs += include_directories('.')
