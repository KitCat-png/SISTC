#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int my_connect(char *servername, char *port);

int main (int argc, char* const argv[]) {
  printf("1170701 - %s\n", __FILE__);
  char buffer[4096];
  
  // ⭐ Why the double \r\n\r\n at the end?
  // In the HTTP protocol, a blank line (Carriage Return + Line Feed) is strictly required to tell the web server "I am done sending headers, you can process the request now."
  char http_msg1[] = "GET /~jes/sistc/pl/ola.html HTTP/1.1\r\n"
                     "Host: ave.dee.isep.ipp.pt\r\n\r\n";
    
  int s = my_connect("ave.dee.isep.ipp.pt", "80");
  
  //Send HTTP request
  write(s, http_msg1, strlen(http_msg1));

  // line-by-line reading
  FILE *fp = fdopen(s, "r");
  if (fp == NULL) {
    perror("fdopen");
    close(s);
    exit(1);
  }

  // Skip HTTP headers
  // ⭐ Why do we need this first loop?
  // When the web server replies, it sends metadata headers first (like 'Content-Type: text/html'), followed by a blank line, then the actual HTML code. 
  // This loop intercepts and throws away the metadata headers until it hits that blank line, so we only print the clean HTML body.
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    if (strcmp(buffer, "\r\n") == 0 || strcmp(buffer, "\n") == 0) {
      break; 
    }
  }

  // print the HTML body
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    fputs(buffer, stdout);
  }
  
  printf("\n\n");
  fclose(fp); // This also closes the socket 's'
  
  return 0;
}

int my_connect(char *servername, char *port) {
  struct addrinfo hints;
  struct addrinfo *addrs;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  int r = getaddrinfo(servername, port, &hints, &addrs);
  if (r != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
    exit(1);
  }
  
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("socket");  
    exit(2);
  }
  
  r = connect(s, addrs->ai_addr, addrs->ai_addrlen);
  if (r == -1) {
    perror("connect");  
    close(s);
    exit(3);
  }
  
  freeaddrinfo(addrs);
  return s;
}
