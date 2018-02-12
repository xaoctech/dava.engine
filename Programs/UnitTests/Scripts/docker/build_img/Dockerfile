FROM docker-registry.k8s/build:centos7

ADD make-src /tmp/make-src
RUN sed -i 's/\r//' /tmp/make-src && chmod 755 /tmp/make-src

ENTRYPOINT /tmp/make-src
