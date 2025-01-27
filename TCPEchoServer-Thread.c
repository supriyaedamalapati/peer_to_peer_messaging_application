#include "TCPEchoServer.h" /* TCP echo server includes */
#include <pthread.h>       /* for POSIX threads */
#include <mysql/mysql.h>   /* for MySQL connection */

MYSQL *conn; /* MySQL connection handle */

/* MySQL table creation query */
const char *CREATE_TABLE_QUERY = "CREATE TABLE IF NOT EXISTS users (id INT AUTO_INCREMENT PRIMARY KEY, username VARCHAR(255) NOT NULL UNIQUE, password VARCHAR(255) NOT NULL, ipaddress VARCHAR(30) NOT NULL, isactive BOOLEAN)";

void *ThreadMain(void *arg); /* Main program of a thread */

/* Structure of arguments to pass to client thread */
struct ThreadArgs
{
  int clntSock; /* Socket descriptor for client */
  char ClntAddr[20];
};

/* Connect to MySQL database */
void ConnectToDB()
{
  /* Initialize MySQL connection */
  conn = mysql_init(NULL);
  if (conn == NULL)
    DieWithError("mysql_init() failed");

  /* Connect to MySQL database */
  if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL)
    DieWithError("Error in Sql connection");

  printf("Connected to MySQL database %s\n", DB_NAME);
}

/* Disconnect from MySQL database */
void DisconnectFromDB()
{
  /* Close MySQL connection */
  mysql_close(conn);
  printf("Disconnected from MySQL database %s\n", DB_NAME);
}

/* Create users table in MySQL database */
void CreateUsersTable()
{
  if (mysql_query(conn, CREATE_TABLE_QUERY) != 0)
    DieWithError("Error in creating table");

  printf("Created table 'users' in MySQL database %s\n", DB_NAME);
}

int main(int argc, char *argv[])
{
  int servSock;                  /* Socket descriptor for server */
  int clntSock;                  /* Socket descriptor for client */
  unsigned short echoServPort;   /* Server port */
  pthread_t threadID;            /* Thread ID from pthread_create() */
  struct ThreadArgs *threadArgs; /* Pointer to argument structure for thread */
  char ClntAddr[20];
  if (argc != 2) /* Test for correct number of arguments */
  {
    fprintf(stderr, "Usage:  %s <SERVER PORT>\n", argv[0]);
    exit(1);
  }

  echoServPort = atoi(argv[1]); /* First arg:  local port */

  servSock = CreateTCPServerSocket(echoServPort);

  ConnectToDB();
  CreateUsersTable();

  for (;;) /* run forever */
  {
    printf("waiting for connection ....\n");
    clntSock = AcceptTCPConnection(servSock, &ClntAddr);
    /* Create separate memory for client argument */
    if ((threadArgs = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs))) ==
        NULL)
      DieWithError("malloc() failed");
    threadArgs->clntSock = clntSock;
    strcpy(threadArgs->ClntAddr, ClntAddr);

    /* Create client thread */
    if (pthread_create(&threadID, NULL, &ThreadMain, (void *)threadArgs) != 0)
      DieWithError("pthread_create() failed");
    printf("with thread %ld\n", (long int)threadID);
  }
  printf("at the end of main thread ....\n");
  /* NOT REACHED */
}

void *ThreadMain(void *threadArgs)
{
  int clntSock; /* Socket descriptor for client connection */
  char clntAddr[20];
  /* Guarantees that thread resources are deallocated upon return */

  pthread_detach(pthread_self());
  printf("In Main thread \n");
  /* Extract socket file descriptor from argument */
  clntSock = ((struct ThreadArgs *)threadArgs)->clntSock;
  strcpy(clntAddr, ((struct ThreadArgs *)threadArgs)->ClntAddr);
  printf("socket %d \n", clntSock);
  printf("address %s \n", clntAddr);

  free(threadArgs); /* Deallocate memory for argument */
  HandleTCPClient(clntSock);
  printf("not called");
  return (NULL);
}
