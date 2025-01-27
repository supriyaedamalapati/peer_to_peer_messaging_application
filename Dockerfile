FROM ubuntu:20.04


RUN apt-get update && apt-get install -y net-tools && apt-get install -y libmysqlclient-dev libssl-dev gcc


ENV LD_LIBRARY_PATH=/usr/local/ssl/lib/
ENV mysql_path=/etc/mysql

WORKDIR /home

COPY TCPEchoServer-Thread.c .
COPY TCPEchoClient.c .
COPY DieWithError.c .
COPY AcceptTCPConnection.c .
COPY CreateTCPServerSocket.c .
COPY HandleTCPClient.c .
COPY TCPClientThread.c .
COPY TCPEchoServer.h .
COPY TCPEchoClient.h .

# RUN apt update
# RUN apt install gcc
RUN gcc -pthread -lssl -lcrypto -l:libssl.so.1.1 -l:libcrypto.so.1.1 -o TCPEchoServer TCPEchoServer-Thread.c DieWithError.c HandleTCPClient.c AcceptTCPConnection.c CreateTCPServerSocket.c $(mysql_config --libs --cflags)
RUN gcc -pthread -lssl -o TCPEchoClient TCPEchoClient.c DieWithError.c TCPClientThread.c

CMD ["bash"]
