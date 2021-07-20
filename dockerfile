FROM gcc:4.9
ENV NAME VAR1
COPY threadserver.sh /threadserver.sh
COPY . /threadserver
WORKDIR /threadserver/
RUN gcc -o threadserver1 threadserver.c -lpthread
WORKDIR /
CMD ["/bin/sh", "/threadserver.sh"]
