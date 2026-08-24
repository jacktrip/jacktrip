# JackTrip debug build — how to capture a crash

This is the same JackTrip as the matching regular Linux release, rebuilt with full
debug symbols so that a crash produces a usable stack trace. It is built with the
same compiler optimisations and the same feature set as the release build — the only
difference is that the debug information needed to read a crash dump is left in the
binary, which is why the file is much larger than usual.

`BUILD-INFO.txt` records exactly which commit it was built from. Please include that
file, or the version it names, in any report.

It needs the same Qt 6 packages as the regular JackTrip Linux build — `INSTALL.md`
in this archive lists them. If your normal JackTrip runs, this one will too.

If you unpacked this with a graphical archive tool, the helper scripts may have lost
their executable bit. Restore it with:

```bash
chmod +x *.sh
```

---

## Option A — run it under gdb (easiest, and gives us the most useful result)

This is the recommended path: gdb catches the crash as it happens and writes both a
readable backtrace and a core file.

```bash
sudo apt install gdb        # or: sudo dnf install gdb
./run-under-gdb.sh
```

This opens the normal JackTrip window, exactly as if you had started JackTrip by
hand — gdb just watches it from the outside. Reproduce the problem the way you
normally hit it, repeating until the crash happens; intermittent crashes can take
several tries.

When it crashes, the script leaves these files in the current directory:

- `jacktrip-gdb-<timestamp>.log` — the backtrace. **This is the important one.**
- `core.<pid>` — the core dump, if gdb managed to write one.

Send us the `.log` file. If a core file is there too, compress it first
(`gzip core.*`) — it is large, and it compresses down a lot.

If JackTrip exits normally, or you stop it with Ctrl-C, the log says so instead of
reporting a crash. Just run the script again.

---

## Option B — plain core dump, no gdb

Use this if you would rather not run under gdb. Ubuntu hands core dumps to apport by
default, which does not keep them anywhere useful for a program that was not
installed from a package, so the core dump destination has to be changed first.

```bash
sudo ./enable-core-dumps.sh      # asks for your password; change is temporary
./run-with-coredump.sh
```

Reproduce the crash. Core files are written to `/tmp/cores/`, named
`core.<program>.<pid>.<timestamp>`.

Send us the core file for `jacktrip` (gzip it first — `gzip /tmp/cores/core.jacktrip.*`)
along with `jacktrip-run-<timestamp>.log`.

The core dump setting reverts on reboot. To undo it right away:

```bash
sudo ./enable-core-dumps.sh --restore
```

---

## What to send back

In rough order of usefulness:

1. `jacktrip-gdb-<timestamp>.log` (Option A) — a backtrace is often enough on its own.
2. The gzipped core file, if you have one.
3. The console log (`jacktrip-run-<timestamp>.log`) — verbose output from the run that
   crashed, which shows how far things got.
4. `BUILD-INFO.txt`, so we know exactly which build produced the crash.
5. What you were doing, whether it had ever worked before, which audio device and
   backend you were using, and roughly how often it happens.

The logs contain JackTrip's normal console output, which names your audio devices and
the studio you connected to. Have a look through them before posting them anywhere
public.

## Notes

- JackTrip's UI is built on QtWebEngine, which runs helper processes. If the crash is
  in one of those rather than in JackTrip itself, you may get a core file named
  `core.QtWebEngineProc...` — that is still useful, please send it.
- Nothing here installs anything or replaces an existing JackTrip installation. It all
  runs from the directory you unpacked it into.

Please report any security concerns to vulnerabilities@jacktrip.org
