#!/usr/bin/env bash
set -e

pod=${1}
container=${2:-debug}

views=(Server Observer Shooter Target)

hosts=(`aws/aws-ips ${dir} ${secret}`)

for view in "${views[@]}"
do
	kubectl cp ${pod}:debug/strikes${view}.log -c ${container} ./game_stats/invaders/logs/strikes${view}.log
done

python ./game_stats/invaders/analyze.py

exit 0
