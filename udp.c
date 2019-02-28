#include "udp.h"

/* requests available stream services from the default 193.136.138.142:59000
if stream service is not specified by user*/
void list_streams(void)
{
  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  int fd, n, addrlen, errcode;
  char buffer[128];
  char host[NI_MAXHOST],service[NI_MAXSERV];//consts in <netdb.h>

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; //IPv4
  hints.ai_socktype = SOCK_DGRAM; //UDP socket
  hints.ai_flags = AI_NUMERICSERV;

  //addr info
  if((errcode = getaddrinfo("193.136.138.142", "59000", &hints, &res))!=0)
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
  if((n=sendto(fd,"DUMP\n",5,0,res->ai_addr,res->ai_addrlen))==-1)
  {
    printf("error: sendto");
    exit(1);
  }

  freeaddrinfo(res);

  //recvfrom
  addrlen = sizeof(addr);
  if((n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen))==-1)
  {
    printf("error: recvfrom");
    exit(1);
  }

  write(1,buffer,n);
  close(fd);

  if((errcode=getnameinfo((struct sockaddr *)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
    fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode));
  else
    printf("sent by [%s:%s]\n",host,service);
}
