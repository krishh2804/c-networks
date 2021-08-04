FROM gcc:4.9
COPY index.html /webserver/
COPY webserver.c /webserver/
WORKDIR /webserver/
RUN gcc -o webserver1 webserver.c
CMD ["./webserver1"]
