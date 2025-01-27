#include "TCPEchoClient.h" /* TCP echo server includes */
#include <stdio.h>         /* for printf() and fprintf() */
#include <sys/socket.h>    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>     /* for sockaddr_in and inet_addr() */
#include <stdlib.h>        /* for atoi() and exit() */
#include <string.h>        /* for memset() */
#include <unistd.h>        /* for close() */
#include <pthread.h>       /* for POSIX threads */

void DieWithError(char *errorMessage); /* Error handling function */

void *receive_thread(void *arg);

struct Client_fd
{
    int sock; /* Socket descriptor for client */
    char echoString[50];
};

#define RCVBUFSIZE 250 /* Size of receive buffer */
void *receive_thread(void *client_fd)
{
    int sock; /* Socket descriptor for client connection */
              /* Guarantees that thread resources are deallocated upon return */
    char echoString[50];
    pthread_detach(pthread_self());

    /* Extract socket file descriptor from argument */
    sock = ((struct Client_fd *)client_fd)->sock;
    strcpy(echoString, ((struct Client_fd *)client_fd)->echoString);
    printf("%d in receive thread \n", sock);
    free(client_fd); /* Deallocate memory for argument */
    TCPClientThreadHandler(sock, echoString);
    return (NULL);
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
    unsigned int echoStringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv() and total bytes read */
    pthread_t clientthreadID;        /* Thread ID from pthread_create() */
    struct Client_fd *client_fd;     /* Pointer to argument structure for thread */
    char buffer[RCVBUFSIZE];

    if ((argc < 3) || (argc > 4)) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n",
                argv[0]);
        exit(1);
    }

    servIP = argv[1];     /* First arg: server IP address (dotted quad) */
    echoString = argv[2]; /* Second arg: string to echo */

    if (argc == 4)
        echoServPort = atoi(argv[3]); /* Use given port, if any */
    else
        echoServPort = 7; /* 7 is the well-known port for the echo service */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
    /* Create separate memory for client argument */
    if ((client_fd = (struct Client_fd *)malloc(sizeof(struct Client_fd))) == NULL)
        DieWithError("malloc() failed");
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");
    /* Send the string to the server */

    printf("%d  sock variable\n", sock);
    client_fd->sock = sock;
    strcpy(client_fd->echoString, echoString);
    printf("%d for struct", client_fd->sock);

    if (pthread_create(&clientthreadID, NULL, &receive_thread, (void *)client_fd) != 0)
        DieWithError("pthread_create() failed");

    printf("with thread %ld\n", (long int)clientthreadID);

    printf("\n"); /* Print a final linefeed */
    pthread_exit(&clientthreadID);
    close(sock);
    printf("Socket closed");
    exit(0);
}
