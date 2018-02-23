#!/usr/bin/env bash

set -e

cp /var/workdir/config.sh /tmp/config.sh
sed -i 's/\r//' /tmp/config.sh
. /tmp/config.sh

for i in `seq 9001 9101`
do
    udppm -d $i 192.168.65.2 9000
done

echo "Proxies started"

while true
do
    sleep 1
done
