FROM docker-registry.k8s/run:centos7

ADD run /tmp/run
RUN sed -i 's/\r//' /tmp/run && chmod 755 /tmp/run

ENTRYPOINT ["/tmp/run"]
