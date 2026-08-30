#!/usr/bin/env bash
# Point kernel core dumps at /tmp/cores instead of apport, which on Ubuntu does not
# keep usable core files for programs that were not installed from a package.
#
#   sudo ./enable-core-dumps.sh            enable
#   sudo ./enable-core-dumps.sh --restore  put the previous setting back
#
# The change is not permanent: it reverts by itself on the next reboot.

set -euo pipefail

BACKUP="/var/tmp/jacktrip-core-pattern.bak"
CORE_DIR="/tmp/cores"
PATTERN="$CORE_DIR/core.%e.%p.%t"

if [ "$(id -u)" -ne 0 ]; then
    echo "This script needs root. Run it as:  sudo $0 ${*:-}" >&2
    exit 1
fi

if [ "${1:-}" = "--restore" ]; then
    if [ -f "$BACKUP" ]; then
        sysctl -w "kernel.core_pattern=$(cat "$BACKUP")" >/dev/null
        rm -f "$BACKUP"
        echo "Restored core_pattern to: $(cat /proc/sys/kernel/core_pattern)"
    else
        echo "No saved setting found. Rebooting also restores the default." >&2
        exit 1
    fi
    exit 0
fi

if [ ! -f "$BACKUP" ]; then
    cat /proc/sys/kernel/core_pattern > "$BACKUP"
fi

mkdir -p "$CORE_DIR"
chmod 1777 "$CORE_DIR"
sysctl -w "kernel.core_pattern=$PATTERN" >/dev/null

echo "Core dumps will now be written to $CORE_DIR/"
echo "  current pattern: $(cat /proc/sys/kernel/core_pattern)"
echo "  previous pattern saved in $BACKUP"
echo
echo "Now run ./run-with-coredump.sh (as your normal user, not with sudo)."
