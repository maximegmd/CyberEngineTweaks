#!/bin/bash
# CET macOS dev helper: parallel build, then a serial pass over what failed so the errors are readable. Summary on stdout.
cd "$(dirname "$0")"
make -C build -k -j10 > build.log 2>&1
make -C build -k -j1 > build1.log 2>&1
rc=$?
echo "exit $rc, $(grep -c 'error:' build1.log) errors"
grep -E 'error:' build1.log | sed -E 's/^.*\/(src|vendor-mac|RED4ext.SDK-macos|macos)\//\1\//' | sort | uniq -c | sort -rn | head -${1:-70}
