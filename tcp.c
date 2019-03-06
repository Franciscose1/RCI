#include "tcp.h"

int reach_tcp(char *ip, char *port)
{
  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  int n, errcode, fd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_STREAM; //TCP socket
  hints.ai_flags = AI_NUMERICSERV;

  //addr info
  if((errcode = getaddrinfo(ip, port, &hints, &res))!=0)
  {
    fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(errcode));
    exit(1);
  }
  //socket
  if((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
  {
    printf("error: socket\n");
    exit(1);
  }
  //connect
  if((n=connect(fd,res->ai_addr,res->ai_addrlen))==-1)
  {
    printf("error: connect tcp\n");
    exit(1);
  }

  return fd;
}

int serv_tcp(char *port)
{
  struct addrinfo hints,*res;
  int fd, addrlen, n, errcode;
  struct sockaddr_in addr;

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_STREAM;//TCP socket
  hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV;

  if((errcode = getaddrinfo(NULL, port, &hints, &res))!=0)
  {
    fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(errcode));
    exit(1);
  }
  if((fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1)
  {
    printf("error: socket\n");
    exit(1);
  }
  if(bind(fd,res->ai_addr,res->ai_addrlen)==-1)
  {
    printf("error: bind tcp\n");
    exit(1);
  }
  if(listen(fd,5)==-1)
  {
    printf("error: listen\n");
    exit(1);
  }
  return fd;
}
