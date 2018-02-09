FROM centos:7
COPY start.sh /bin/start.sh
RUN sed -i 's/\r//' /bin/start.sh && yum install -y epel-release && yum install -y 3proxy
CMD /bin/start.sh
