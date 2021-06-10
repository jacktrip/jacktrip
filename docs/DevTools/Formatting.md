# Formatting with clang-format

JackTrip uses clang-format to specify the Code Formatting Style.
Most IDEs are able to recognize the `.clang-format` file in JackTrip's project
directory and integrate the formatting capabilities.

Formatting specific files is done like this:

```bash
clang-format -i class.cpp class.h
```

This overrides all given files. If you only want to check the formatting `-i` has
to be removed.

Areas in the source code that must not be formatted have to be embraced in following statements:

```c++
this = is.formatted;
// clang-format off
this+=   isnot     ;
// clang-format on
this = again;
```

## Formatting the entire code base

Formatting the entire code base can be done with specifying all code files of the project
or be invoking Meson's clang-format target:

```bash
ninja -C builddir clang-format
```

