#!/usr/bin/env bash

# How to use tc:
# https://wiki.linuxfoundation.org/networking/netem
# if UDP 'duplex connection' is used then params applays twice

# Examples

cmd=${1:-add}

tc qdisc show | grep netem > /dev/null
exit_code=$?

if [ "${cmd}" == "del" ]; then
    if [ "${exit_code}" -eq 0 ]; then
        tc qdisc del dev eth0 root
    fi
fi

if [ "${cmd}" == "add" ]; then
    if [ "${exit_code}" -eq 0 ]; then
        cmd="change"
    fi
    tc qdisc ${cmd} dev eth0 root netem delay 50ms 50ms distribution normal loss 3%
fi
