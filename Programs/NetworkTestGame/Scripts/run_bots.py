import pip

try:
 import psutil
except ImportError, e:
 pip.main(['install', "psutil"])

import argparse
import time
import psutil

usage = """
run_bots.py --run --count BOTS_COUNT --path PATH_TO_CLIENT_EXECUTABLE [OPTIONS]
or
run_bots.py --stop
"""
description = "Run or stop bot clients."
parser = argparse.ArgumentParser(usage=usage, description=description)

parser.add_argument("--run", help="Run bots", action='store_true')
parser.add_argument("--stop", help="Stop all bots", action='store_true')
parser.add_argument("--count", help="Set number of bots to start", default=1, type=int)
parser.add_argument("--path", help="Path to client executable")
parser.add_argument("--server-ip", help="Server IP address, 127.0.0.1 by default", default="127.0.0.1")
parser.add_argument("--server-port", help="Server port, 9000 by default", default=9000, type=int)
parser.add_argument("--client-token", help="Client machine id, added to client token", type=int)


args = parser.parse_args()

if (args.run and args.stop) or not (args.run or args.stop):
	print "Either --run or --stop must be set"
	exit(7)

if args.stop:
	killed = 0
	for proc in psutil.process_iter():
		if proc.name() == "Client" or proc.name() == "Client.exe":
			proc.kill()
			killed += 1
	print "Killed %d processes" % killed
	exit(0)

if args.run:
	if not args.path:
		print "--path must be set"
		exit(7)
	print "Running %d client with options %s" % (args.count, [args.path, '--host', args.server_ip, '--port', str(args.server_port), '--token TOKEN', '--bot', 'random'])
	for i in range (1, args.count+1):
		token = str(i)
		if args.client_token:
			token += str(args.client_token)
		commandline = [args.path, '--host', args.server_ip, '--port', str(args.server_port), '--token', token, '--bot', 'random', '--log']
		print commandline
		psutil.Popen(commandline)
		time.sleep(2)
