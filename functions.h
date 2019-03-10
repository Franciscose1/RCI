#ifndef FUNCTIONS
#define FUNCTIONS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ON 1
#define OFF 0
#define max(A,B) ((A)>=(B)?(A):(B))

struct USER{
  char stream_name[128];
  char stream_addr[128];
  char stream_port[128];
  char ipaddr[128];
  char tport[128]; //Clientes
  char uport[128]; //Servidor de acesso
  char rsaddr[128];
  char rsport[128];
  int tcpsessions;
  int bestpops;
  int tsecs;
  int display;
  int detailed_info;
  int synopse;
  enum {access_server,out,in,waiting} state;
  int fd_udp_serv, fd_tcp_serv, fd_tcp_mont, *fd_clients;
  char **myClients;
};

typedef struct USER User;

//String handling
int str_to_msgID(char *, char *);
int str_to_IP_PORT(char *, char *, char *);
int str_to_streamID(char *, char *, char *, char *);
//Setup do programa
void USER_init(User *);
int read_args(int, char **, User *);
//Implementação do protocolo de comunicação
void msg_in_protocol(char *, char *, User *);
int handle_RSmessage(char *, User *);
int handle_ASmessage(char *, User *);
int handle_STDINmessage(char *, User *);
int handle_PEERmessage(char *, User *);
//Mecanismo de adesão à árvore
int join_tree(User *);

#endif
