import sys
import os
import shutil
import argparse
from subprocess import check_call

_DF_PATH='/tmp/dava.framework'
_BUILD_PATH = _DF_PATH + '/Programs/NetworkTestGame/_docker'
_ROOT_DIR = os.getcwd() + '/../../..'

parser = argparse.ArgumentParser()

parser.add_argument('--image', help='image name with tag (default name is dev:1)', default='dev:1')
parser.add_argument('--with-push', help='push the generated image to private Docker registry', action='store_true')
parser.add_argument('--with-breakpad', help='generate and upload debug symbols', action='store_true')
parser.add_argument('--gen-k8s', metavar='FILENAME', help='generate a config file for Kubernetes', default='')
args = parser.parse_args()

name, tag = args.image.split(':') # the tag we can use as a version
build_args = ''
if args.with_breakpad:
    build_args += '--build-arg WITH_BREAKPAD=yes --build-arg VERSION={0}'.format(tag)

shutil.rmtree('_game', ignore_errors=True)
script = (
    'docker build {0} -t build-img docker/build_img'.format(build_args),
    'docker run -i -v {0}:{1} --name build-cnt build-img'.format(_ROOT_DIR, _DF_PATH),
    'docker cp build-cnt:{0} _game'.format(_BUILD_PATH),
    'docker rm build-cnt',
    'docker rmi build-img',
    'docker run --rm -i -v {0}:{1} centos rm -rf {2}'.format(_ROOT_DIR, _DF_PATH, _BUILD_PATH),
    'docker build -t {0} -f docker/run_img/Dockerfile .'.format(args.image),
    'docker tag {0} docker-registry.k8s/{0}'.format(args.image),
)
for cmd in script:
    print cmd
    check_call(cmd.split())

if args.with_push:
    check_call('docker push docker-registry.k8s/{0}'.format(args.image).split())

if args.gen_k8s:
    tag = int(tag)
    config = '''
apiVersion: v1 
kind: Pod
metadata:
  name: {0}
  labels:
    app: {0}
spec:
  restartPolicy: Never
  volumes:
  - name: shared-data
    emptyDir: {{}}
  containers:
  - name: dev
    image: docker-registry.k8s/{0}:{1}
    imagePullPolicy: Always
    resources:
      limits:
        cpu: "1.3"
      requests:
        cpu: "1.1"
    volumeMounts:
    - name: shared-data
      mountPath: /debug
    ports:
    - containerPort: 9000
    args: ["--game", "default", "--health-check-host", "localhost", "--health-check-port", "5050", "--game-stats-log", "/debug/game-stats.log"]
    securityContext:
      privileged: true
  - name: debug
    image: docker-registry.k8s/debug:latest
    volumeMounts:
    - name: shared-data
      mountPath: /debug
  - name: healthz
    image: docker-registry.k8s/healthz:4
    resources:
      limits:
        cpu: "0.2"
      requests:
        cpu: "0.1"
    ports:
    - containerPort: 8080
    livenessProbe:
      httpGet:
        path: /healthz
        port: 8080
      initialDelaySeconds: 5
      periodSeconds: 5

---
kind: Service
apiVersion: v1
metadata:
  name: {0}-service
spec:
  selector:
    app: {0}
  type: NodePort
  ports:
  - name: game
    protocol: UDP
    port: 9000
'''.format(name, tag)
    with open(args.gen_k8s, 'w') as config_file:
        config_file.write(config)
