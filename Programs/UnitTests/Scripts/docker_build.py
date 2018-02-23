import sys
import os
import shutil
from subprocess import check_call

_DF_PATH='/tmp/dava.framework'
_ROOT_DIR = os.getcwd() + '/../../..'

script = (
    'docker build -t build-img docker/build_img',
    'docker run --rm -i -v {0}:{1} --name build-cnt build-img'.format(_ROOT_DIR, _DF_PATH),
    'docker build -t run-img docker/run_img',
    'docker run --rm -i -v {0}:{1} --name run-cnt run-img'.format(_ROOT_DIR, _DF_PATH),
    'docker rmi build-img',
    'docker rmi run-img'
)
for cmd in script:
    print cmd
    check_call(cmd.split())
