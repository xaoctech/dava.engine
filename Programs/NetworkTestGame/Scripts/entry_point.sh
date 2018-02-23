#!/ust/bin/env bash

ulimit -c unlimited

mkdir -p /cores
echo '/debug/core_%e.%p' > /proc/sys/kernel/core_pattern

$@
