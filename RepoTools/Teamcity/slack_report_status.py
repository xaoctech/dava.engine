import pip

try:
 import requests
except ImportError, e:
 pip.main(['install', "requests"])

import requests
import argparse

arg_parser = argparse.ArgumentParser()

arg_parser.add_argument('result', choices=['success', 'failure'], help='Build result.')
arg_parser.add_argument('--teamcity-env-build-failed', default = 'true', choices=['true', 'false'], help='If true, "successful" result is ignored. This flag is needed because teamcity does not support "run if build failed" build step. [env.build_failed]')
arg_parser.add_argument('--build-id', type=int, required=True, help='Teamcity unique build-id, used to generate link to build. [teamcity.build.id]')
arg_parser.add_argument('--configuration-name', required=True, help='Teamcity configuration name. [system.teamcity.buildConfName]')
args = arg_parser.parse_args()

if args.result == 'failure' and args.teamcity_env_build_failed == 'false':
	exit(0)

if args.result == 'success':
	color = "#00D000"
	string_result = "Success"
else:
	color = "#D00000"
	string_result = "Failure"

r = requests.post('https://hooks.slack.com/services/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', timeout = 10, json={"text": "%s: %s" % (args.configuration_name, string_result),
	"attachments": 
	[
	{
		"fallback": string_result,
		"color": color,
		"fields": [
		{
			"title": string_result ,
			"value": "<https://teamcity2.wargaming.net/viewLog.html?buildId=%d|Build link>" % args.build_id,
			"short": True
		}
		]
	}
	]})