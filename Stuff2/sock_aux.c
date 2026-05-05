#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

void print_address(const struct sockaddr * clt_addr, socklen_t addrlen) 
{
  char hostname[256];  
  char port[6];

  int n = getnameinfo(clt_addr, addrlen, hostname, sizeof(hostname), 
    port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV); 
  if(n != 0)
    printf("%s\n", gai_strerror(n));
  else
    printf("Connection from %s:%s\n", hostname, port);
}

void print_socket_address(int sd)
{
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  getsockname(sd, (struct sockaddr *) &addr, &addrlen);

  print_address((struct sockaddr *) &addr, addrlen);
}


int my_create_server_socket(char *port)
{
  int s, r;
  struct addrinfo hints, *a;

  memset (&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_flags = AI_PASSIVE;
  r = getaddrinfo(NULL, port, &hints, &a);
  if (r != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
    exit(1);
  } 


  s = socket(PF_INET, SOCK_STREAM, 0);
  if(s == -1){
    perror("socket");
    exit(1);
  }


  //avoid bind errors if the port is still being used by previous connections
  int so_reuseaddr = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));

  
  r = bind(s, a->ai_addr, a->ai_addrlen);
  if(r == -1) {
    perror("bind");
    exit(1);
  }    
  
  r = listen(s, 5); 
  if(r == -1) {
    perror("listen");
    exit(1);
  }   
  
  freeaddrinfo(a);
  return s;
}

int my_connect(char *servername, char *port) {

  //get server address using getaddrinfo
  struct addrinfo hints;
  struct addrinfo *addrs;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  int r = getaddrinfo(servername, port, &hints, &addrs);
  if (r != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
    return -1;
  }
  
  //create socket
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("socket");  
    return -1;
  }
  
  //connect to server
  r = connect(s, addrs->ai_addr, addrs->ai_addrlen);
  if (r == -1) {
    perror("connect");  
    close(s);
    return -1;
  }
 
  freeaddrinfo(addrs);  
  return s;
}


int myReadBlock(int s, void *buf, int count)
{
  int r, nread = 0;

  while( nread < count ) {
    r = read(s, buf+nread, count-nread);
    if(r <= 0)
      break;
    else
      nread += r;
  }
  
  return nread;  
}

int myReadLine1(int s, char *buf, int count){
  int r, n=0;

  if(count<=0)
    return 0;
  else if(count==1) {
      buf[0] = 0;
      return 0;
    } else
      --count; //leave space for '\0'

  do {
    r = read(s, buf+n, 1);
    if(r==1)
      ++n;
  } while( r==1 && n < count && buf[n-1]!='\n');
  
  buf[n] = '\0';  
  
  return n;  
}


int myReadLine2(int s, char *buf, int count) {
  static int lbuf_n = 0; //number of bytes stored in lbuf
  static char lbuf[4096];
  int nproc, n = 0;

  if(count<=0)
    return 0;
  else if(count==1) {
      buf[0] = 0;
      return 0;
    } else
      --count;
  
  do {
    nproc=0;
    if(lbuf_n) {
      do {
        buf[n] = lbuf[nproc];
        ++n;
        ++nproc;
      } while( nproc!=lbuf_n && n < count && buf[n-1]!='\n');
      
      lbuf_n = lbuf_n - nproc;
      //move the remaining bytes in lbuf to the beginning
      if(lbuf_n)
        memcpy(lbuf, lbuf+nproc, lbuf_n);

      if( n == count || buf[n-1]=='\n' )
        break;
    }

    lbuf_n = read(s, lbuf, sizeof(lbuf));

  } while(lbuf_n>0);

  buf[n] = 0; 

  return n;  
}

