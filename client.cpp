#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

int main(int argc, char* argv[])
{
   int nSocket;
   int nConnect;
   int nBytes;
   struct sockaddr_in stServerAddr;
   struct hostent* lpstServerEnt;
   char cbBuf[BUF_SIZE];

   if (argc != 3)
   {
     fprintf(stderr, "Usage: %s server_name port_number\n", argv[0]);
     exit(1);
   }

   /* look up server's IP address */
   lpstServerEnt = gethostbyname(argv[1]);
   if (! lpstServerEnt)
   {
      fprintf(stderr, "%s: Can't get the server's IP address.\n", argv[0]);
      exit(1);
   }

   /* create a socket */
   nSocket = socket(PF_INET, SOCK_STREAM, 0);
   if (nSocket < 0)
   {
      fprintf(stderr, "%s: Can't create a socket.\n", argv[0]);
      exit(1);
   }

   /* server info */
   memset(&stServerAddr, 0, sizeof(struct sockaddr));
   stServerAddr.sin_family = AF_INET;
   memcpy(&stServerAddr.sin_addr.s_addr, lpstServerEnt->h_addr, lpstServerEnt->h_length);
   stServerAddr.sin_port = htons(atoi(argv[2]));

   /* connect to the server */
   nConnect = connect(nSocket, (struct sockaddr*)&stServerAddr, sizeof(struct sockaddr));
   if (nConnect < 0)
   {
      fprintf(stderr, "%s: Can't connect to the server (%s:%i).\n", argv[0], argv[1], atoi(argv[2]));
      exit(1);
   }

   /* connection is now established; read from socket */
   nBytes = read(nSocket, cbBuf, BUF_SIZE);
   write(1, cbBuf, nBytes);

   close(nSocket);
   return 0;
}