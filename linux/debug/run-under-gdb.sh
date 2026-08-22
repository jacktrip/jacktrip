#!/usr/bin/env bash
# Run the JackTrip debug build under gdb so that a crash produces a readable
# backtrace (and a core file, if gdb can write one).
#
# Usage: ./run-under-gdb.sh [extra jacktrip arguments]

set -uo pipefail
cd "$(dirname "$(readlink -f "$0")")"

if ! command -v gdb >/dev/null 2>&1; then
    echo "gdb is not installed. Install it with:  sudo apt install gdb" >&2
    exit 1
fi

TIMESTAMP=$(date +%Y%m%d-%H%M%S)
LOG="jacktrip-gdb-$TIMESTAMP.log"

# Allow gdb to write a core file of unlimited size.
ulimit -c unlimited

echo "Logging to $LOG"
echo "Reproduce the problem now: connect to a studio as you normally would."
echo "If it does not crash, quit JackTrip and run this script again."
echo

gdb -q -batch -x crash.gdb --args ./jacktrip -V "$@" 2>&1 | tee "$LOG"

echo
echo "==================================================================="
if grep -q "CRASH DETAILS BELOW" "$LOG"; then
    echo "A crash was captured."
else
    echo "No crash this time — JackTrip exited on its own."
fi
echo "Saved log: $(pwd)/$LOG"
if ls core.* >/dev/null 2>&1; then
    echo "Core file(s):"
    ls -lh core.* | awk '{print "    "$9"  ("$5")"}'
    echo "Please gzip the core file before sending it:  gzip core.*"
else
    echo "No core file was written (that is OK — the log is the important part)."
fi
echo "Please send the log file back to us."
echo "==================================================================="
