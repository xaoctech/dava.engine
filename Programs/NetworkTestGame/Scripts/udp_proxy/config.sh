#!/usr/bin/env bash

# How to use tc:
# https://wiki.linuxfoundation.org/networking/netem
# if UDP 'duplex connection' is used then params applays twice

# Examples
tc qdisc add dev eth0 root netem delay 50ms 50ms distribution normal loss 3%
