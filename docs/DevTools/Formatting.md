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

## Forget about formatting with a git pre-commit hook

With git the user can install hooks that are executed when specific tasks are done.
We can add a pre-commit hook for clang-format. So that everytime we commit our
changes, git runs clang-format for us.

Handling git hooks by hand is cumbersome. With [pre-commit](https://pre-commit.com/)
this becomes a lot easier.

Pre-commit is a Python app that can be installed with pip.

```bash
pip install pre-commit
```

Within the root directory of the jacktrip repository the pre-commit hook can be
installed as follows:

```bash
pre-commit install
```

Pre-commit only runs on changed files. Running pre-commit on all files is done
by:

```bash
pre-commit run --all-files
```

Sometimes these hooks come into your way. But you can disable them when committing:

```bash
git commit -am "Commit all my stuff" --no-verify
```

### Pre-commit configuration

Pre-commit is configured by the *.pre-commit-config.yaml* file in the repository's
root. Currently it only includes the clang-format hook.

```yaml
fail_fast: false
repos:
  - repo: https://github.com/ssciwr/clang-format-hook
    rev: v13.0.1
    hooks:
    - id: clang-format
      files: ^src/
      types_or: [c++]
      exclude: '^src/.+dsp\.h'
```
