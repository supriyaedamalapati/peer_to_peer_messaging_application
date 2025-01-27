
# Peer-to-peer Messaging and Broadcast Application

This is a C language based application that allows users to communicate with each other through peer-to-peer messaging and broadcasting. The application uses a TCP socket for peer-to-peer communication and a UDP socket for broadcasting messages.

Features
- Users can sign up and log in using a username and password.
- The username and password are stored in a MySQL database with the password being hashed for security.
- After logging in, users are prompted with two options: display all active users or broadcast messages.
- Users can select their desired option and communicate through peer-to-peer messaging or broadcasting.

Required Packages
gcc - 9.4.0
mysql - 8.0.32
mysqlclient-dev
openssl library


Steps to Execute the Project:

- Start the server in port 4400 by running the following command in a terminal:

gcc -lpthread -lssl -lcrypto -l:libssl.so.1.1 -l:libcrypto.so.1.1 -o TCPEchoServer TCPEchoServer-Thread.c DieWithError.c HandleTCPClient.c AcceptTCPConnection.c CreateTCPServerSocket.c $(mysql_config --libs --cflags)

Then run this command in the same terminal:

./TCPEchoServer 4400

- Start the client by running the following command in a new terminal:

gcc -pthread -lssl -o TCPEchoClient TCPEchoClient.c DieWithError.c TCPClientThread.c

- Then run this command in the same terminal to connect to the server:

./TCPEchoClient 127.0.0.1 "Hello" 4400

In this command, "127.0.0.1" represents the server's localhost, "Hello" is a string to notify the server that a client is connecting, and "4400" is the port number on which the server is running.

Screenshorts :

![Broadcast ,message](https://user-images.githubusercontent.com/55336660/236057128-f349193a-7d9b-47ef-b0c5-c9ebb28fd77e.png)
![Signup snip](https://user-images.githubusercontent.com/55336660/236057294-e1ee8775-3285-45b5-ab26-0a8713b3c5a8.png)

