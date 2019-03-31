#include "udp.h"

/* requests available stream services from the default 193.136.138.142:59000
if stream service is not specified by user*/
int reach_udp(char *ip, char *port, char *msg)
{
  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  int fd, n, errcode, counter, maxfd;
  unsigned int addrlen;
  char buffer[128] = {'\0'};
  char host[NI_MAXHOST],service[NI_MAXSERV];//consts in <netdb.h>
  struct timeval tv = {10, 0};
  fd_set rfds;

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

  FD_SET(fd,&rfds); maxfd = fd;

  counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL, &tv);
  if(counter<0){printf("Counter < 0\n");exit(1);}

  if(counter == 0)
  {
    printf("recvfrom timed out\n");
    return 0;
  }

  //recvfrom
  addrlen = sizeof(addr);
  if((n=recvfrom(fd,buffer,128,0,(struct sockaddr*)&addr,&addrlen))==-1)
  {
    printf("error: recvfrom\n");
    exit(1);
  }
  strcpy(msg,buffer);
  //write(1,buffer,n);
  close(fd);


  if((errcode=getnameinfo((struct sockaddr *)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
    fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode));


  return 1;
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

int recieveNsend_udp(int fd, char *msg, User *user)
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
    printf("error: recvfrom\n");
    return 0;
  }
  strcpy(msg,buffer);
  if(user->detailed_info == ON) write(1,buffer,n);

  //Mostra quem mandou (não é pedido)
  if((errcode=getnameinfo((struct sockaddr *)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
    fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode));
  else
  if(user->detailed_info == ON) printf("Sent by [%s:%s]\n",host,service);

  //Processa o pedido feito ao servidor de acesso
  handle_ASmessage(buffer,user);

  //Responde de acordo
  n = strlen(buffer);
  n=sendto(fd,buffer,n,0,(struct sockaddr*)&addr,addrlen);
  if(n==-1){printf("error: sendto\n"); return 0;} //Exit se falhar, tem de ser mudado

  return 1;
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
