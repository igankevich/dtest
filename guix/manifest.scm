(packages->manifest
  (append
    (list (@ (gnu packages build-tools) meson)
          (@ (gnu packages ninja) ninja)
          (@ (gnu packages cmake) cmake)
          (@ (gnu packages pkg-config) pkg-config)
          (@ (gnu packages commencement) gcc-toolchain)
          (list (@ (gnu packages gcc) gcc) "lib")
          (@ (gnu packages check) googletest)
          (@ (gnu packages unistdx) unistdx)
          (@ (gnu packages compression) zlib)
          (@ (gnu packages compression) xz)
          (@ (gnu packages pre-commit) python-pre-commit)
          (@ (gnu packages python) python-3)
          (list (@ (gnu packages llvm) clang-10) "extra") ;; clang-tidy
          )))
