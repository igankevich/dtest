dtest_lib_src = files([
    'application.cc',
    'cluster.cc',
    'cluster_node_bitmap.cc',
    'exit_code.cc',
])

dtest_lib_deps = [unistdx,threads]

dtest_lib = library(
    'dtest',
    sources: dtest_lib_src,
    include_directories: src,
    dependencies: dtest_lib_deps,
    implicit_include_directories: false,
    version: meson.project_version(),
    install: true,
)

dtest = declare_dependency(
    link_with: dtest_lib,
    include_directories: src,
    dependencies: dtest_lib_deps,
)

pkgconfig.generate(
    dtest_lib,
    requires: dtest_lib_deps,
    version: meson.project_version(),
    name: 'dtest',
    filebase: 'dtest',
    description: project_description,
)

dtest_src = files([
    'main.cc',
    'python.cc',
])

dtest_exe = executable(
    'dtest',
    sources: dtest_src,
    include_directories: src,
    dependencies: [dtest,python3],
    implicit_include_directories: false,
    install: true,
)

install_headers(
    'application.hh',
    'cluster.hh',
    'cluster_node.hh',
    'cluster_node_bitmap.hh',
    'exit_code.hh',
    subdir: meson.project_name()
)

clang_tidy_files += dtest_lib_src
clang_tidy_files += dtest_src
