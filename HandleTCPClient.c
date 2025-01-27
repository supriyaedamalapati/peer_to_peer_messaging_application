#include "TCPEchoServer.h" /* TCP echo server includes */
#include <stdio.h>         /* for printf() and fprintf() */
#include <sys/socket.h>    /* for recv() and send() */
#include <unistd.h>        /* for close() */
#include <string.h>
#include <mysql/mysql.h> /* for MySQL connection */
#include <openssl/sha.h>

MYSQL *conn; /* MySQL connection handle */

#define RCVBUFSIZE 300 /* Size of receive buffer */
#define USERNAME_SIZE 100
#define PASSWORD_SIZE 100
#define MAX_ROWS 100
#define MAX_COLS 100

void HashPassword(char password[PASSWORD_SIZE], char *hex_hashed_password)
{
    unsigned char hashed_password[SHA256_DIGEST_LENGTH];

    char *password_to_hash = password;
    SHA256((unsigned char *)password_to_hash, strlen(password_to_hash), hashed_password);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(hex_hashed_password + 2 * i, "%02x", hashed_password[i]);
    }
}

void InsertDataInUsers(char username[USERNAME_SIZE], char password[PASSWORD_SIZE], char clntAddr[20])
{
    char hex_hashed_password[2 * SHA256_DIGEST_LENGTH + 1];
    HashPassword(password, hex_hashed_password);

    ConnectToDB();
    char query[200];
    sprintf(query, "insert into users(username,password,ipaddress,isactive) values('%s','%s', '%s',FALSE);", username, hex_hashed_password, clntAddr);
    printf("%s \n", query);
    if (mysql_query(conn, query))
        printf("Unable to insert data into Employee table\n");
    DisconnectFromDB();
}

void SelectDataFromUsers(char username[USERNAME_SIZE], char password[PASSWORD_SIZE], char **data)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    int num_fields, i, j;

    char query[100];
    ConnectToDB();
    sprintf(query, "select password from users where username='%s';", username);
    printf("%s \n", query);
    if (mysql_query(conn, query))
    {
        DieWithError("Error in executing SQL select query");
    }

    result = mysql_store_result(conn);

    if (result)
    {
        if ((row = mysql_fetch_row(result)))
        {
            *data = row[0];
        }
    }
    else
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_free_result(result);
    DisconnectFromDB();
}

void SetUserActive(char username[USERNAME_SIZE])
{
    ConnectToDB();
    char query[200];
    sprintf(query, "update users set isactive=TRUE where username='%s';", username);
    printf("%s \n", query);
    if (mysql_query(conn, query))
        printf("Unable to insert data into Employee table\n");
    DisconnectFromDB();
}
void SetUserInactive(char username[USERNAME_SIZE])
{
    ConnectToDB();
    char query[200];
    sprintf(query, "update users set isactive=FALSE where username='%s';", username);
    printf("%s \n", query);
    if (mysql_query(conn, query))
        printf("Unable to insert data into Employee table\n");
    DisconnectFromDB();
}

void ShowActiveClients(int clntSocket, char *column1, char *column2)
{

    MYSQL_RES *result;
    MYSQL_ROW row;
    int num_fields, i, j;
    char query[100];
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize;

    ConnectToDB();

    sprintf(query, "select username, ipaddress from users where isactive=TRUE;");

    printf("%s \n", query);
    if (mysql_query(conn, query))
    {
        DieWithError("Error in executing SQL select query");
    }
    printf("Executing SQL \n");
    result = mysql_store_result(conn);
    int rows = mysql_num_rows(result);
    int numFields = mysql_num_fields(result);
    char totalRows[100];
    sprintf(totalRows, "%d", rows);

    printf("socket %d \n", clntSocket);

    char promptMessage[] = "ActiveClients";
    if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
        DieWithError("send() failed");

    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed");
    echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
    printf("%s", echoBuffer);
    if (result)
    {

        if (send(clntSocket, totalRows, sizeof(totalRows), 0) != sizeof(totalRows))
            DieWithError("send() failed");
        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

        while ((row = mysql_fetch_row(result)))
        {
            column1 = row[0];
            column2 = row[1];
            printf("%s is %s years old\n", column1, column2);

            if (send(clntSocket, column1, sizeof(column1), 0) != sizeof(column1))
                DieWithError("send() failed");

            if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                DieWithError("recv() failed");
            echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
            if (send(clntSocket, column2, sizeof(column2) + 2, 0) != sizeof(column2) + 2)
                DieWithError("send() failed");
            if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                DieWithError("recv() failed");
            echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
        }
    }
    else
    {
        printf("error in sql \n");
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    mysql_free_result(result);
    DisconnectFromDB();
}

void BroadcastSender(char BroadcastMessage[200])
{
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast address */
    char *broadcastIP;                /* IP broadcast address */
    unsigned short broadcastPort;     /* Server port */
    char *sendString;                 /* String to broadcast */
    int broadcastPermission;          /* Socket opt to set permission to broadcast */
    unsigned int sendStringLen;       /* Length of string to broadcast */

    broadcastIP = "127.0.0.1"; /* First arg:  broadcast IP address */
    broadcastPort = 4401;      /* Second arg:  broadcast port */
    sendString = BroadcastMessage;      /* Third arg:  string to broadcast */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Set socket to allow broadcast */
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *)&broadcastPermission,
                   sizeof(broadcastPermission)) < 0)
        DieWithError("setsockopt() failed");

    /* Construct local address structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));       /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                     /* Internet address family */
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP); /* Broadcast IP address */
    broadcastAddr.sin_port = htons(broadcastPort);          /* Broadcast port */

    sendStringLen = strlen(sendString); /* Find length of sendString */
    for (;;)                            /* Run forever */
    {
        /* Broadcast sendString in datagram to clients every 3 seconds*/
        if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) != sendStringLen)
            DieWithError("sendto() sent a different number of bytes than expected");

        sleep(3); /* Avoids flooding the network */
    }
    /* NOT REACHED */
}
void DieWithError(char *errorMessage); /* Error handling function */
void Login(int clntSocket)
{
    char promptMessage[200];
    int attempt = 0;
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize;
    char *column1;
    char *column2;
    char hex_hashed_password[2 * SHA256_DIGEST_LENGTH + 1];
    char *data[MAX_ROWS * MAX_COLS];
    char BroadcastMessage[200];

    while (true)
    {
        strcpy(promptMessage, "Enter username: ");
        if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");

        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed");
        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

        strcpy(username, echoBuffer);

        strcpy(echoBuffer, "");

        strcpy(promptMessage, "Enter Password: ");
        if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
            DieWithError("send() failed");

        if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed");
        echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

        strcpy(password, echoBuffer);

        SelectDataFromUsers(username, password, data);
        printf("Hashed password: %s\n", *data);

        HashPassword(password, hex_hashed_password);
        if (strcmp(hex_hashed_password, *data) == 0)
        {
            /*Send Login success message to client*/
            strcpy(promptMessage, "LoginSuccess");
            if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
                DieWithError("send() failed");
            SetUserActive(username);

            if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                DieWithError("recv() failed");
            echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
            switch (atoi(echoBuffer))
            {
            case 1:
                printf("Entered Switch \n");
                ShowActiveClients(clntSocket, column1, column2);
                break;
            case 2:
                if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
                    DieWithError("recv() failed");
                echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
                strcpy(BroadcastMessage, echoBuffer);
                BroadcastSender(BroadcastMessage);
                break;
            default:
                SetUserInactive(username);
                break;
            }
            break;
        }
        else
        {
            if (attempt == 2)
                DieWithError("User authentication failed limit reached.");
            attempt = attempt + 1;

            strcpy(promptMessage, "Login Attempt failed");
            if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
                DieWithError("send() failed");
            echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
        }
    }
}

void Signup(int clntSocket)
{
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    char confirmpassword[PASSWORD_SIZE];
    char promptMessage[200];
    int recvMsgSize;             /* Size of received message */
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    char clntAddr[20] = "127.0.0.1";

    strcpy(promptMessage, "Enter username: ");
    if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
        DieWithError("send() failed");

    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed");
    echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

    strcpy(username, echoBuffer);

    strcpy(promptMessage, "Enter Password: ");
    if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
        DieWithError("send() failed");

    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed");
    echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
    strcpy(password, echoBuffer);

    strcpy(promptMessage, "Confirm Password: ");
    if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
        DieWithError("send() failed");

    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed");
    echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
    strcpy(confirmpassword, echoBuffer);

    InsertDataInUsers(username, password, clntAddr);

    Login(clntSocket);
}

void HandleTCPClient(int clntSocket)
{
    printf("In Handletcpclient");
    char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
    int recvMsgSize;             /* Size of received message */
    char hashed_password[PASSWORD_SIZE];
    char choice[10];

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) < 0)
        DieWithError("recv() failed");
    echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */

    printf("%s \n", echoBuffer);
    /* Send prompt message to client */
    char promptMessage[] = "Welcome! Please select your choice as 1 or 2. \n 1. login \n 2. signup\n";
    if (send(clntSocket, promptMessage, sizeof(promptMessage), 0) != sizeof(promptMessage))
        DieWithError("send() failed");

    if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
        DieWithError("recv() failed");
    echoBuffer[recvMsgSize] = '\0'; /* Terminate the string! */
    strcpy(choice, echoBuffer);

    printf("Label reached");
    switch (atoi(echoBuffer))
    {
    case 1:
        Login(clntSocket);
        break;
    case 2:
        Signup(clntSocket);
        break;
    default:
        close(clntSocket);
        break;
    }
    close(clntSocket); /* Close client socket */
}
