#!/usr/bin/env bash

set -e

count=${1:-97}
host=${2:-127.0.0.1}
port=${3:-10101}
host_id=${4:-1}
bot_kind=${5:-random}
log_stats=${6:-false}

rm -f *.log

for i in $(seq 1 ${count}); do
    stdout="bot_${i}-out.log"
    stderr="bot_${i}-err.log"
    token=${i}${host_id}
    ${log_stats} && stats_log_file="game-stats-${token}.log" || stats_log_file=""
    ${BOT_APP} --slow-down 0.03 --host ${host} --port ${port} --token ${token} --bot ${bot_kind} --game-stats-log ${stats_log_file} </dev/null 1>${stdout} 2>${stderr} &
    sleep 3
done
