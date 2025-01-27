#include "TCPEchoClient.h" /* TCP echo server includes */
#include <stdio.h>         /* for printf() and fprintf() */
#include <sys/socket.h>    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>     /* for sockaddr_in and inet_addr() */
#include <stdlib.h>        /* for atoi() and exit() */
#include <string.h>        /* for memset() */
#include <unistd.h>        /* for close() */
#include <pthread.h>       /* for POSIX threads */
#include <sys/time.h>

#define RCVBUFSIZE 250 /* Size of receive buffer */
#define USERNAME_SIZE 100
#define PASSWORD_SIZE 100
#define MAXRECVSTRING 255 /* Longest string to receive */

void sending();
void receiving(int client_fd);

char *cName;
unsigned short cPort;
char *cIP;

/* Structure of arguments to pass to client thread */
struct ThreadArgs
{
    int clntSock; /* Socket descriptor for client */
    char ClntAddr[20];
};
// Calling receiving every 2 seconds
void *receive_thread_handler(void *client_fd)
{
    int s_fd = *((int *)client_fd);
    while (1)
    {
        sleep(2);
        receiving(s_fd);
    }
}

void *BroadcastReceiver()
{
    int sock;
    struct sockaddr_in broadcastAddr;   /* Broadcast Address */
    unsigned short broadcastPort;       /* Port */
    char recvString[MAXRECVSTRING + 1]; /* Buffer for received string */
    int recvStringLen;                  /* Length of received string */

    broadcastPort = 4401; /* First arg: broadcast port */

    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));  /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    broadcastAddr.sin_port = htons(broadcastPort);     /* Broadcast port */

    /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0)
        DieWithError("bind() failed");

    /* Receive a single datagram from the server */
    if ((recvStringLen = recvfrom(sock, recvString, MAXRECVSTRING, 0, NULL, 0)) < 0)
        DieWithError("recvfrom() failed");

    recvString[recvStringLen] = '\0';
    printf("BroadCast Message Received: %s \n", recvString); /* Print the received string */

    close(sock);
    exit(0);
}

// Sending messages to port
void sending()
{

    char buffer[2000] = {0};
    // Fetching port number
    char ip_send[40];
    int port_send;

    // IN PEER WE TRUST
    printf("Enter the IP address to send message:");
    scanf("%s", ip_send);
    printf("Enter the port to send message:");
    scanf("%d", &port_send);

    int sock = 0, valread;
    struct sockaddr_in send_addr;
    char hello[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("Socket creation error");
    }

    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = inet_addr(ip_send);
    send_addr.sin_port = htons(port_send);

    if (connect(sock, (struct sockaddr *)&send_addr, sizeof(send_addr)) < 0)
    {
        DieWithError("Connection Failed");
    }

    char dummy;
    printf("Enter your message:");
    scanf("%c", &dummy); // The buffer is our enemy
    scanf("%[^\n]s", hello);
    sprintf(buffer, "%s[PORT:%d] says: %s", cName, cPort, hello);
    send(sock, buffer, sizeof(buffer), 0);
    printf("\nMessage sent\n");
    close(sock);
}

// Receiving messages on our port
void receiving(int client_fd)
{
    struct sockaddr_in address;
    int valread;
    char buffer[2000] = {0};
    int addrlen = sizeof(address);
    fd_set current_sockets, ready_sockets;

    // Initialize my current set
    FD_ZERO(&current_sockets);
    FD_SET(client_fd, &current_sockets);
    int k = 0;
    while (1)
    {
        k++;
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            DieWithError("Error");
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {

                if (i == client_fd)
                {
                    int client_socket;

                    if ((client_socket = accept(client_fd, (struct sockaddr *)&address,
                                                (socklen_t *)&addrlen)) < 0)
                    {
                        DieWithError("accept");
                    }
                    FD_SET(client_socket, &current_sockets);
                }
                else
                {
                    valread = recv(i, buffer, sizeof(buffer), 0);
                    printf("\n%s\n", buffer);
                    FD_CLR(i, &current_sockets);
                }
            }
        }

        if (k == (FD_SETSIZE * 2))
            break;
    }
}
void P2PMessage()
{
    cName = (char *)malloc(20);
    cIP = (char *)malloc(20);

    strcpy(cName, "revanth");
    strcpy(cIP, "127.0.0.1");
    cPort = 4400;

    int client_fd, new_socket, valread;
    struct sockaddr_in address;
    int k = 0;

    // Creating socket file descriptor
    if ((client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        DieWithError("socket() failed");
    }
    // Forcefully attaching socket to the port

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(cIP);
    address.sin_port = htons(cPort);

    if (bind(client_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        DieWithError("bind failed");
    }
    if (listen(client_fd, 5) < 0)
    {
        DieWithError("listen");
    }
    int ch;
    pthread_t tid;
    pthread_create(
        &tid, NULL, &receive_thread_handler,
        &client_fd); // Creating thread to keep receiving message in real time
    printf("\n*****At any point in time press the following:*****\n1.Send "
           "message\n0.Quit\n");
    printf("\nEnter choice:");
    do
    {

        scanf("%d", &ch);
        switch (ch)
        {
        case 1:
            sending();
            break;
        case 0:
            printf("\nLeaving\n");
            break;
        default:
            printf("\nWrong choice\n");
        }
    } while (ch);

    close(client_fd);
}

void ShowActiveClients(int sock, char echoBuffer[RCVBUFSIZE])
{

    int bytesRcvd;
    char promptMessage[150];
    int recMSGSize;
    char column1[100];
    char column2[100];

    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
    printf("%s \n", echoBuffer);
    if (strcmp(echoBuffer, "ActiveClients") == 0)
    {
        strcpy(promptMessage, "Send clients");
        if (send(sock, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");

        if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0) // Total no of rows
            DieWithError("recv() failed or connection closed prematurely");
        echoBuffer[bytesRcvd] = '\0';

        int totalRows = atoi(echoBuffer);
        for (int i = 0; i < totalRows; i++)
        {
            if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                DieWithError("recv() failed or connection closed prematurely");
            echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */

            strcpy(column1, echoBuffer);
            strcpy(promptMessage, "Column1Received");

            if (send(sock, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
                DieWithError("send() failed");

            if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                DieWithError("recv() failed or connection closed prematurely");
            echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
            strcpy(column2, echoBuffer);
            printf("%s \n", echoBuffer);
            strcpy(promptMessage, "Column2Received");
            if (send(sock, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
                DieWithError("send() failed");
            printf("%s ip address is %s \n", column1, column2);
        }
    }
}

void Login(int sock)
{
    char echoBuffer[RCVBUFSIZE];
    int bytesRcvd;
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char choice[10];
    char loginSuccessMessage[25] = "LoginSuccess";
    char p2paddress[20];
    char broadcastmessage[200];
    pthread_t tid;

    /* Receive the same string back from the server */
    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
    printf("%s \n", echoBuffer);  /* Print the echo buffer */
    /*Enter username*/
    scanf("%s", username);

    if (send(sock, username, strlen(username), 0) != strlen(username))
        DieWithError("send() sent a different number of bytes than expected");

    /*Receive Password message from Handler*/
    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
    printf("%s", echoBuffer);     /* Print the echo buffer */
    /*Enter password*/
    scanf("%s", password);
    /*send Password to Handler*/
    if (send(sock, password, strlen(password), 0) != strlen(password))
        DieWithError("send() sent a different number of bytes than expected");

    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */

    if (strcmp(echoBuffer, loginSuccessMessage) == 0)
    {
        printf("%s \n", echoBuffer);

        printf("Select any one of below options.\n 1. List of active users for P2P communication \n 2.Broadcast a message \n 3. Logout \n 4. Listen to broadcast messages   \n");
        scanf("%s", choice);

        if (send(sock, choice, strlen(choice), 0) != strlen(choice))
            DieWithError("send() sent a different number of bytes than expected");

        switch (atoi(choice))
        {
        case 1:
            ShowActiveClients(sock, echoBuffer);
            printf("Select any peer ip address to start P2P communication");
            scanf("%s", p2paddress);
            break;
        case 2:
            printf("Enter the broadcast message:");
            scanf("%s", broadcastmessage);
            if (send(sock, broadcastmessage, strlen(broadcastmessage), 0) != strlen(broadcastmessage))
                DieWithError("send() sent a different number of bytes than expected");
            break;
        case 4:
            pthread_create(&tid, NULL, BroadcastReceiver, NULL); // Creating thread to keep receiving message in real time
        default:
            close(sock);
            break;
        }
    }
}


void Signup(int sock)
{
    int bytesRcvd;
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char confirmPassword[PASSWORD_SIZE];
    char echoBuffer[RCVBUFSIZE];

    /* Receive the same string back from the server */
    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
    printf("%s", echoBuffer);     /* Print the echo buffer */
    /*Enter username*/
    scanf("%s", username);

    if (send(sock, username, strlen(username), 0) != strlen(username))
        DieWithError("send() sent a different number of bytes than expected");

    /*Receive Password message from Handler*/
    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
    printf("%s", echoBuffer);     /* Print the echo buffer */
    /*Enter password*/
    scanf("%s", password);
    /*send Password to Handler*/
    if (send(sock, password, strlen(password), 0) != strlen(password))
        DieWithError("send() sent a different number of bytes than expected");
    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
    printf("%s", echoBuffer);     /* Print the echo buffer */
    /*Enter confirmPassword*/
    scanf("%s", confirmPassword);
    /*send Password to Handler*/
    if (send(sock, confirmPassword, strlen(confirmPassword), 0) != strlen(confirmPassword))
        DieWithError("send() sent a different number of bytes than expected");
    printf("Signup Successful, Please login to client \n");
    Login(sock);
}

void TCPClientThreadHandler(int sock, char echoString[50])
{
    char choice[10];
    char echoBuffer[RCVBUFSIZE];   /* Buffer for echo string */
    unsigned int echoStringLen;    /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd; /* Bytes read in single recv() and total bytes read */
    printf("In TCPClientthread %d \n", sock);
    echoStringLen = strlen(echoString);
    if (send(sock, echoString, echoStringLen, 0) != echoStringLen)
        DieWithError("send() sent a different number of bytes than expected");
    /* Receive the same string back from the server Ask for login/sign choice */
    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed or connection closed prematurely");
    echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
    printf("%s", echoBuffer);     /* Print the echo buffer */
    scanf("%s", choice);

    /* Send the choice to the server */

    if (send(sock, choice, strlen(choice), 0) != strlen(choice))
        DieWithError("send() sent a different number of bytes than expected");
    switch (atoi(choice))
    {
    case 1:
        Login(sock);
        break;
    case 2:
        Signup(sock);
        break;
    default:
        close(sock);
        break;
    }
}
