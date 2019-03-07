#include "udp.h"

/* requests available stream services from the default 193.136.138.142:59000
if stream service is not specified by user*/
void reach_udp(char *ip, char *port, char *msg)
{
  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  int fd, n, errcode;
  unsigned int addrlen;
  char buffer[128] = {'\0'}, msgID[128] = {'\0'};
  char host[NI_MAXHOST],service[NI_MAXSERV];//consts in <netdb.h>

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_DGRAM; //UDP socket
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
  //sendto
  if((n=sendto(fd,msg,strlen(msg),0,res->ai_addr,res->ai_addrlen))==-1)
  {
    printf("error: sendto\n");
    exit(1);
  }
  //free addrinfo structure
  freeaddrinfo(res);

  //recvfrom
  addrlen = sizeof(addr);
  if((n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen))==-1)
  {
    printf("error: recvfrom\n");
    exit(1);
  }
  strcpy(msg,buffer);
  write(1,buffer,n);
  close(fd);

  if((errcode=getnameinfo((struct sockaddr *)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
    fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode));
  else
    printf("Sent by [%s:%s]\n",host,service);
}

int send_udp(char *ip, char *port, char *msg)
{
  struct addrinfo hints, *res;
  int fd, n, errcode;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_DGRAM; //UDP socket
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
    printf("error: socket");
    exit(1);
  }
  //sendto
  if((n=sendto(fd,msg,strlen(msg),0,res->ai_addr,res->ai_addrlen))==-1)
  {
    printf("error: sendto");
    exit(1);
  }
  //free addrinfo structure
  freeaddrinfo(res);

  return fd;
}

void recieve_udp(int fd, char *msg)
{
  struct sockaddr_in addr;
  int n, errcode;
  unsigned int addrlen;
  char buffer[128] = {'\0'};
  char host[NI_MAXHOST],service[NI_MAXSERV];//consts in <netdb.h>

  //recvfrom
  addrlen = sizeof(addr);
  if((n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen))==-1)
  {
    printf("error: recvfrom");
    exit(1);
  }
  strcpy(msg,buffer);
  write(1,buffer,n);
  close(fd);

  if((errcode=getnameinfo((struct sockaddr *)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
    fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode));
  else
    printf("Sent by [%s:%s]\n",host,service);
}

int serv_udp(char *port)
{
  struct addrinfo hints,*res;
  int fd,n,nread,errcode;
  char buffer[128];

  memset(&hints,0,sizeof hints);
  hints.ai_family=AF_INET;//IPv4
  hints.ai_socktype=SOCK_DGRAM;//UDP socket
  hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV;

  if((errcode = getaddrinfo(NULL, port, &hints, &res))!=0)
  {
    fprintf(stderr, "error: getaddrinfo: %s\n", gai_strerror(errcode));
    exit(1);
  }
  if((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1)
  {
    printf("error: socket udp\n");
    exit(1);
  }
  if((n=bind(fd,res->ai_addr,res->ai_addrlen))==-1)
  {
    printf("error: bind udp\n");
    exit(1);
  }
  freeaddrinfo(res);

  return fd;
}
