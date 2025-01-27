
# Peer-to-peer Messaging and Broadcast Application

This project demonstrates a multithreaded TCP server-client architecture with MySQL database integration and secure user authentication. It includes functionalities for user login, signup, and broadcast communication, with passwords securely hashed using SHA256.

---

## Features

- **Multithreaded TCP Server**: Handles multiple clients concurrently using POSIX threads.
- **User Authentication**:
  - **Signup**: Allows users to create accounts with a username and password.
  - **Login**: Authenticates users by securely comparing SHA256-hashed passwords stored in the database.
- **MySQL Integration**: 
  - Stores user credentials securely in a database.
  - Dynamically creates the `users` table if it doesn't exist.
- **Broadcast Communication**: Server broadcasts a message periodically to all clients.
- **Secure Communication**: Passwords are hashed using OpenSSL's SHA256 before storing them in the database.

---

## Project Structure

- `TCPEchoServer.h`  
  Header file containing constants, function declarations, and buffer sizes.

- `CreateTCPServerSocket.c`  
  Contains logic for creating and binding a TCP server socket.

- `AcceptTCPConnection.c`  
  Handles incoming client connections.

- `HandleTCPClient.c`  
  Handles client-server interaction, including login, signup, and broadcasting.

- `TCPEchoClient.c`  
  Client-side implementation for connecting to the server and interacting with it.

- `TCPEchoServer-Thread.c`  
  Main server file implementing a multithreaded architecture to handle multiple clients.

---

## Prerequisites

1. **Libraries**:  
   Ensure the following libraries are installed on your system:
   - OpenSSL (for SHA256 hashing)
   - MySQL development libraries (for database integration)
   - pthread (for multithreading)

2. **Database Setup**:
   - Create a MySQL database named `mydb` (or update `DB_NAME` in the code).
   - Use the following credentials or update them in the code:
     - **Host**: `localhost`
     - **User**: `debian-sys-maint`
     - **Password**: `y5AyLlBo4PUnqOTJ`

---

## Setup and Usage

### Server

1. **Compile the server code**:
   ```bash
   gcc -o server CreateTCPServerSocket.c AcceptTCPConnection.c HandleTCPClient.c TCPEchoServer-Thread.c -lmysqlclient -lssl -lpthread
