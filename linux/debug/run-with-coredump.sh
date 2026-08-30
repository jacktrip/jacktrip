#!/usr/bin/env bash
# Run the JackTrip debug build with core dumps enabled and verbose logging.
# Run ./enable-core-dumps.sh (with sudo) once before using this.
#
# Usage: ./run-with-coredump.sh [extra jacktrip arguments]

set -uo pipefail
cd "$(dirname "$(readlink -f "$0")")"

PATTERN=$(cat /proc/sys/kernel/core_pattern)
case "$PATTERN" in
    /*) : ;;
    *)
        echo "WARNING: core dumps are still being sent to apport:"
        echo "    $PATTERN"
        echo "A crash will most likely NOT leave a core file behind."
        echo "Run 'sudo ./enable-core-dumps.sh' first, or use ./run-under-gdb.sh instead."
        echo
        read -r -p "Continue anyway? [y/N] " reply
        [ "$reply" = "y" ] || [ "$reply" = "Y" ] || exit 1
        ;;
esac

ulimit -c unlimited

# JackTrip only starts its normal window when it is given no options at all, so
# --gui is added explicitly. Extra arguments are still honoured.
if [ "$#" -eq 0 ]; then
    set -- --gui
fi

TIMESTAMP=$(date +%Y%m%d-%H%M%S)
LOG="jacktrip-run-$TIMESTAMP.log"

echo "Core dump pattern: $PATTERN"
echo "Logging to $LOG"
echo "Reproduce the crash now (connect to a studio)."
echo

./jacktrip "$@" 2>&1 | tee "$LOG"
STATUS=${PIPESTATUS[0]}

echo
echo "==================================================================="
echo "JackTrip exited with status $STATUS"
if [ "$STATUS" -ge 128 ]; then
    echo "That looks like a crash (signal $((STATUS - 128)))."
fi
echo "Saved log: $(pwd)/$LOG"
case "$PATTERN" in
    /*) CORE_DIR=$(dirname "$PATTERN") ;;
    *)  CORE_DIR="/tmp/cores" ;;
esac
CORES=$(ls -t "$CORE_DIR"/core.* 2>/dev/null | head -5)
if [ -n "$CORES" ]; then
    echo "Recent core files:"
    ls -lh $CORES | awk '{print "    "$9"  ("$5")"}'
    echo "Please gzip the newest one before sending it:  gzip <path>"
else
    echo "No core file found in $CORE_DIR/."
fi
echo "==================================================================="
