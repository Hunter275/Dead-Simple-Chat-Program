/*

Hunter Thornsberry
Cameron Chorba
Derek Fish
David Flook
Emma LaCroix

Version: Awesome Alpaca
Source: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html

basicsocketclient.c - The client side of the client/server socket program. Ask the users for a message to pass over the socket to the server.

*/
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_BUF 1024
#define h_addr h_addr_list[0] /* Backwards compatability http://stackoverflow.com/questions/11405819/does-struct-hostent-have-a-field-h-addr */

void error(char *msg) { /* Error handeling method, simply prints the error and exits */
  perror(msg);
  exit(1);
}


int readPipe()
{
  int fd;
  char * myfifo = "/tmp/myfifo";
  char buf[MAX_BUF];
  /* open, read, and display the message from the FIFO */
  while (1) {
    fd = open(myfifo, O_RDONLY);
    read(fd, buf, MAX_BUF);
    printf("Received: %s \n", buf);
  }
  close(fd);
  return 0;
}

int main(int argc, char *argv[]) {
  int sockfd, port, n;

  struct sockaddr_in server_addr;
  struct hostent *server;

  char buffer[256]; /* Our buffer is 256, everything needs to fit into that */
  if (argc < 3) { /* Check that all the arguments were passed */
    fprintf(stderr, "Useage %s hostname port\n", argv[0]);
    exit(0);
  }
  port = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("Error opening socket");
  }
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr, "Error, no such host\n");
    exit(0);
  }
  bzero((char *) &server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length); /* WHAT DOES THIS LINE DO? */
  server_addr.sin_port = htons(port);
  if (connect(sockfd,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    error("Error connectiong");
  }
  while (1) {
    printf("Please enter a message: ");
    bzero(buffer, 256); /* Set the buffer to all 0s (so as to clear the message) */
    fgets(buffer, 255, stdin); /* Get the message, input it into buffer */
    n = write(sockfd, buffer, strlen(buffer)); /* Write to the socket */
    if (n < 0) {
      error("Error writing to socket");
    }
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) {
      error("Error reading from socket");
    }
    printf("Server: %s\n", buffer);
  }
}
