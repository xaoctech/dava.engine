#!/usr/bin/env bash

set -e

COUNT=${1:-97}
HOST=${2:-127.0.0.1}
PORT=${3:-10101}
CLIENT=${4:-1}
BOT_KIND=${5:-random}

for i in $(seq 1 ${COUNT}); do
    stdout="bot_${i}-out.log"
    stderr="bot_${i}-err.log"
    ${BOT_APP} --slow-down 0.03 --host ${HOST} --port ${PORT} --token ${i}${CLIENT} --bot ${BOT_KIND} </dev/null 1>${stdout} 2>${stderr} &
    sleep 3
done
