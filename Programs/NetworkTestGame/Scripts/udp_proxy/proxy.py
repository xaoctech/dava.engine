import os
import sys
from subprocess import check_call

_IMAGE = 'udp-proxy'
_CONTAINER = 'udp-proxy-cnt'

def _exec(cmd):
    print cmd
    check_call(cmd.split())

def build():
    _exec('docker build -t {0} .'.format(_IMAGE))

def run():
    _exec('docker run --privileged -d -v {0}:/var/workdir -p 9001-9101:9001-9101/udp --name {1} {2}'.format(os.getcwd(), _CONTAINER, _IMAGE))

def stop():
    _exec('docker stop {0}'.format(_CONTAINER))
    _exec('docker rm {0}'.format(_CONTAINER))

def attach():
    _exec('docker exec -it --privileged {0} /bin/bash'.format(_CONTAINER)) 

def logs():
    _exec('docker logs {0}'.format(_CONTAINER))

def usage():
    print 'Usage:\n\t$>python proxy.py build|run|stop|attach|logs'
    sys.exit(1)

if __name__ == '__main__':
    cmds = {
        'build': build,
        'run': run,
        'stop': stop,
        'attach': attach,
        'logs': logs
    }
    if len(sys.argv) != 2 or sys.argv[1] not in cmds:
        usage()
    else:
        cmds[sys.argv[1]]()
