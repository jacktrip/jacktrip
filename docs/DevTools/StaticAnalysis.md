# Static Analysis

Static analysis tools usually needs a `compile_commands.json` file.
If you use Meson to build JackTrip, it already created this file in your build
directory.

## Scan-Build

If you only want to run basic static analysis on JackTrip, you can install
scan-build. Meson will automatically generate a scan-build target for you:

```bash
ninja -C builddir scan-build
```

This should never show any warnings or errors.

## Clang-Tidy

If you installed [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) and
there's a `.clang-tidy` file in the project directory,
Meson generates a clang-tidy target, too. 
The `.clang-tidy` file defines which [checks](https://clang.llvm.org/extra/clang-tidy/checks/list.html)
should be run.
Running clang-tidy on the entire code base:

```bash
ninja -C builddir clang-tidy
```

In most cases you want to run clang-tidy on single files.

```bash
cd src
run-clang-tidy.py -j 4 -p ../builddir singlefile.cpp
```

In `-j #`, `#` specifies the number of tidy instances that run in parallel.

### Fixes

For some checks clang-tidy offers automatic fixes. Some of these are save to apply
without worries. Others should be checked afterwards.

```bash
run-clang-tidy.py -fix -j 4 -p ../builddir singlefile.cpp
```

