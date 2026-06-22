#!/usr/bin/env bash
# test.sh — load the module, exercise it via tool, unload it.
set -e

KO="kernel/cyclic_buffer.ko"
TOOL="userspace/tool"

echo "== make all =="
make all
 
if [ ! -f "$KO" ]; then
    echo "Build did not produce $KO - check kernel/Makefile"
    exit 1
fi
if [ ! -x "$TOOL" ]; then
    echo "Build did not produce $TOOL -check  userspace/Makefile"
    exit 1
fi

echo "-- load --"
trap 'echo "-- unload --"; make unload' EXIT
make load

sleep 0.2
echo "-- dmesg (last lines) --"
sudo dmesg | tail -5

echo "-- device node --"
ls -la /dev/cyclic_buffer

echo "-- buffer_size --"
cat /sys/module/cyclic_buffer/parameters/buffer_size


echo "== tool: clear =="
"$TOOL" clear

SYSFS="/sys/class/cyclic_buffer/cyclic_buffer"

echo "== sysfs: initial stats =="
cat "$SYSFS/bytes_written"
cat "$SYSFS/bytes_read"
cat "$SYSFS/fill_percent"

echo "== tool: write =="
"$TOOL" write "hello kernel"

echo "== sysfs: after write =="
cat "$SYSFS/bytes_written"
cat "$SYSFS/fill_percent"

echo "== tool: avail =="
"$TOOL" avail

echo "== tool: read =="
"$TOOL" read 12

echo "== sysfs: after read =="
cat "$SYSFS/bytes_read"
cat "$SYSFS/fill_percent"

echo "== tool: avail (after read) =="
"$TOOL" avail

echo "Done."