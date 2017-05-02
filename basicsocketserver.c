/*

Hunter Thornsberry
Cameron Chorba
Derek Fish
David Flook
Emma LaCroix

Version: Cautious Canary

Sources:
* Sockets: http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
* curl: http://stackoverflow.com/questions/13905774/in-c-how-do-you-use-libcurl-to-read-a-http-response-into-a-string

Notes:
* A simple node.js server is running on localhost:8080 that supplies Date.now() as the content of the webpage.
  Time is in UNIX time format and is compared with the system time. This would be done between two different machines in a client/server format but is done locally for testing.

* There is residual code from attempts to use Pipes for Interprocess Communication, which would be used for more than one person chatting at a time (chat rooms)

basicsocketserver.c - Ran in conjuction with basicsocketclient.c, this is the server side of the client/server socket relationship. This program deals mainly with setting up the Internet Socket, and passing messages across it.

*/

/* define our libraries */
#include <fcntl.h> /* function control */
#include <stdio.h> /* standard I/O */
#include <sys/stat.h> /* stat stuff (don't think it's used, but was in example code) http://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/stat.h.html */
#include <sys/types.h> /* used for size_t and size of memory http://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/types.h.html */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* INTERNET sockets */
#include <unistd.h> /* was in example code http://pubs.opengroup.org/onlinepubs/7908799/xsh/unistd.h.html */
#include <string.h> /* strings */
#include <stdlib.h> /* used for memory alloc */
#include <curl/curl.h> /* cURL https://curl.haxx.se/libcurl/c/libcurl.html */
#include <time.h> /* system time */


/* define our functions */
void verifyInput(int agrc); /* validates that the hostname and port were entered */
void createSocket(int port); /* creates the socket */
void sendMessage(int sock, char message[255]); /* sends a message */
void makeConnect(int sock); /* connects with socket */
void checkTime(); /* Checks that the clocks of the API server (http://localhost:8080) and the server are on the same relative time (+/- 1min UNIX TIME) */


struct url_data { /* our URL struct used for getting the info from the webpage (http://localhost:8080) */
  size_t size;
  char* data;
};


int pfds[2];

void error(char *msg) { /* Error handeling method, simply prints the error and exits */
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[]) {
  int port;
  checkTime();
  verifyInput(argc);
  port = atoi(argv[1]); /* cast our argument as an int (a-to-i) */
  createSocket(port);
  checkTime();
  return 0;
}

void verifyInput(int argc) { /* Verify the arguments */
  if (argc < 2) { /* check input for the arguments */
    fprintf(stderr, "ERROR, please use: ./server <port> \n");
    exit(1);
  }
}

void createSocket(int port) { /* Creates the internet socket */
  int sockfd, newsockfd, pid;
  socklen_t client;
  char buf[30];
  struct sockaddr_in server_addr, client_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0); /* init our socket */
  if (sockfd < 0) {
    error("ERROR opening socket");
  }
  bzero((char *) &server_addr, sizeof(server_addr)); /* places "0" bytes */
  server_addr.sin_family = AF_INET; /* Internet socket */
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port); /* Used for address translation https://www.systutorials.com/docs/linux/man/3p-htons/ */
  if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    error("Error on binding, port already in use?");
  }
  listen(sockfd, 5); /* Start listeing on port */
  client = sizeof(client_addr);
  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client);
    if (newsockfd < 0) {
      error("Error on accept");
    }
    pipe(pfds);
    pid = fork(); /* Fork the processes, this is used because each connection needs a PID */
    /*
    if (fork()) {
      write(pfds[1], "test", 5); *//* attempt to write "Test" to our pipe NOT WORKING *//*
    } */
    /*
    read(pfds[0], buf, 5); *//* read from the pipe NOT WORKING *//*
    sendMessage(newsockfd, buf); *//* send our message accross the socket */
    if (pid < 0) {
      error("Error on fork");
    }
    if (pid == 0) {
      close(sockfd);
      makeConnect(newsockfd);
      exit(0);
    }
    else {
      close(newsockfd);
    }
  }
}

void sendMessage(int sock, char message[255]) { /* Writes to the socket */
  write(sock, message, strlen(message)); /* Write to the socket the contents of the message and the length */
}

void makeConnect(int sock) { /* Allow connections to our server */
  while (1) {
    int n;
    int fd;
    char buf[30];
    char buffer[256]; /* Our buffer is an array of size 256, every message will need to fit in the buffer */
    char output[1024];
    char * myfifo = "/tmp/myfifo"; /* First In First Out (FIFO) pipe */
    bzero(buffer, 256);
    n = read(sock, buffer, 255);
    if (n < 0) {
      error("Error reading from socket");
    }
    printf("Mesage: %s\n", buffer);
    write(pfds[1], buffer, sizeof(buffer));
    n = write(sock, "Recieved: ", 10);
    sendMessage(sock, buffer);
    if (n < 0) {
      error("Error writing to socket");
    }
  }
}


size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data) { /* This is the curl data writing method to convert the UNIX time response from the webserver (see Sources)*/
  size_t index = data->size; /* This method allocates and deallocates data steams from the webserver into readable format */
  size_t n = (size * nmemb);
  char* tmp;

  data->size += (size * nmemb);

  tmp = realloc(data->data, data->size + 1);

  if (tmp) {
    data->data = tmp;
  }
  else {
    if (data->data) {
      free(data->data);
    }
    return 0; /* failed */
  }
  memcpy((data->data + index), ptr, n);
  data->data[data->size] = '\0';

  return size * nmemb;
}

char *handle_url() { /* curl doing it's thing (see Sources) */
    /* init data types and variables */
    CURL *curl;
    CURLcode res;
    struct url_data data; /* our url data struct used here */
    data.size = 0;
    data.data = malloc(4096); /* set up the buffer */
    if(NULL == data.data) { /* verify that we have our data to write to */
        fprintf(stderr, "Failed to allocate memory.\n");
        return NULL;
    }

    data.data[0] = '\0';
    curl = curl_easy_init(); /* Libraries are great */
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080"); /* open the URL */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data); /* callback function to write data (see above) */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data); /* callback function to write data (see above) */
        res = curl_easy_perform(curl); /* response from the webserver (UNIX time) */
        if(res != CURLE_OK) { /* Check if our response is valid */
                fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
        }
        curl_easy_cleanup(curl); /* close the connection, clear the data, clean up curl */
    }
    return data.data; /* our response data */
}

void checkTime(void) { /* http://stackoverflow.com/questions/13905774/in-c-how-do-you-use-libcurl-to-read-a-http-response-into-a-string */
  char* data;
  char formattedNow[13];
  char subFormattedNow[10];
  char subData[10];
  time_t now = time(0);
  printf("Current System UNIX Time:         ");
  sprintf(formattedNow, "%d\n", now); /* convert now into a string */
  data = handle_url("http://localhost:8080"); /* get the stuff from the webpage */
  if (data) {
    printf("%s \n", data);
    strncpy(subData, data, 9);
    strncpy(subFormattedNow, formattedNow, 9);
    subData[9] = '\0'; /* null terminator */
    subFormattedNow[9] = '\0'; /* null terminator */
    printf("Current Webserver Time Allowance: %sXXXX \n", subData);
    if (strncmp(subData, subFormattedNow, 10)) {
      printf("Incorrect time between server and webserver, please adjust system time. \n");
      exit(1);
    }
    free(data);
  }
}
