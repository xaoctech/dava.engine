FROM docker-registry.k8s/run:centos7

ADD entry_point.sh /bin/entry_point.sh
ADD _game/app /bin/app
ENV LD_LIBRARY_PATH=/bin/app

ENTRYPOINT ["/bin/bash", "/bin/entry_point.sh", "/bin/app/Server"]
