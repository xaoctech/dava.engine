FROM docker-registry.k8s/build:centos7

ARG WITH_BREAKPAD=no
ENV WITH_BREAKPAD ${WITH_BREAKPAD}

ARG VERSION=nover
ENV VERSION ${VERSION}

ADD make-src /tmp/make-src
RUN sed -i 's/\r//' /tmp/make-src && chmod 755 /tmp/make-src

ENTRYPOINT /tmp/make-src
