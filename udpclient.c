//udp client
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

int main(void)
{
  struct addrinfo hints, *res;
  int fd, n;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_DGRAM; //UDP socket
  hints.ai_flag = AI_NUMERICSERV;

  if((errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", "58001", &hints, &res))!=0)
  {
    fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(errcode));
    exit(1);
  }
  if((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
  {
    printf("error: socket");
    exit(1);
  }
  if((n=sendto(fd,"Hello!\n",7,0,res->ai_addr,res->ai_addrlen))==-1)
  {
    printf("error: socket");
    exit(1);
  }
  freeaddrinfo(res);
  exit(0);
}
