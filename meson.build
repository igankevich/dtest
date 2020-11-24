project(
    'dtest',
    'cpp',
    version: '0.1.0',
    meson_version: '>=0.50.0',
    default_options: ['cpp_std=c++11'],
    license: 'gpl3+'
)

project_description = 'Unit tests for distributed applications'

with_debug = get_option('with_debug')

cpp = meson.get_compiler('cpp')

cpp_args = [
    '-Werror=return-type',
    '-Werror=return-local-addr',
    '-Werror=cast-align',
    '-Wcast-align=strict'
]
cpp_link_args = ['-rdynamic']
cpp_args += '-fvisibility-inlines-hidden'

foreach arg : cpp_args
    if cpp.has_argument(arg)
        add_global_arguments(arg, language: 'cpp')
    endif
endforeach

foreach arg : cpp_link_args
    if cpp.has_link_argument(arg)
        add_global_link_arguments(arg, language: 'cpp')
    endif
endforeach

threads = dependency('threads')
unistdx_version = '>=0.41.0'
unistdx = with_debug ? dependency('unistdx-debug', version: unistdx_version) : dependency('unistdx', version: unistdx_version)
gtest = dependency('gtest', main: true)
python3 = dependency('python3-embed')

src = include_directories('src')
pkgconfig = import('pkgconfig')
clang_tidy_files = []

subdir('src')
