#include "functions.h"
#include "udp.h"
#include "tcp.h"

int str_to_msgID(char *ptr, char *msgID)
{
  int n = 0, ncount = 0;

  if(sscanf(ptr, "%s%n", msgID, &n)==1)
  {
    //printf("%s\n", stream_name);
    ptr += n; /* advance the pointer by the number of characters read */
    ncount += n;
    if ((*ptr != ' ')&&(*ptr != '\n'))
    {
      printf("Incompatible with protocol\n");
      return 0;
      //strcpy(flag,"BAD_ID");
    }
    ncount++;
  }else{
    printf("Failed to read msg_ID\n");
    return 0;
  }
  return ncount;
}

int str_to_IP_PORT(char *ptr, char *stream_name, char *ipaddr, char *port, char *flag)
{
  int n = 0, ncount = 0;

  if(strcmp(flag,"streamID")==0)
  {
    if(sscanf(ptr, "%[^:]%n", stream_name, &n)==1)
    {
      //printf("%s\n", stream_name);
      ptr += n; /* advance the pointer by the number of characters read */
      ncount += n;
      if ( *ptr != ':' )
      {
        printf("Incompatible with streamID protocol\n");
        strcpy(flag,"BAD_ID");
        return 0;
      }
      ptr++;
      ncount++;
    }else{
      printf("Failed to read stream_name\n");
      strcpy(flag,"BAD_ID");
      return 0;
    }
  }
  if(sscanf(ptr, "%[^:]%n", ipaddr, &n)==1)
  {
    //printf("%s\n", stream_addr);
    ptr += n; /* advance the pointer by the number of characters read */
    ncount += n;
    if ( *ptr != ':' )
    {
      printf("Incompatible with addr protocol\n");
      strcpy(flag,"BAD_ID");
      return 0;
    }
    ptr++;
    ncount++;
  }else{
    printf("Failed to read addr\n");
    strcpy(flag,"BAD_ID");
    return 0;
  }
  if(sscanf(ptr, "%s%n", port, &n)==1)
  {
    //printf("%s\n", stream_port);
    ptr += n; /* advance the pointer by the number of characters read */
    ncount += n;
    if ( *ptr == '\0' || *ptr == '\n' || *ptr == ' ')
    {

    }else{
      printf("Incompatible with port protocol\n");
      strcpy(flag,"BAD_ID");
    }
    ncount++;
  }else{
    printf("Failed to read port\n");
    strcpy(flag,"BAD_ID");
    return 0;
  }
  return ncount;
}

void USER_init(User *user)
{
  strncpy(user->stream_name,"\0",sizeof(user->stream_name)-1);
  strncpy(user->stream_addr,"\0",sizeof(user->stream_addr)-1);
  strncpy(user->stream_port,"\0",sizeof(user->stream_port)-1);
  strncpy(user->ipaddr,"\0",sizeof(user->ipaddr)-1);
  strncpy(user->tport,"58000",sizeof(user->tport)-1);
  strncpy(user->uport,"58000",sizeof(user->uport)-1);
  strncpy(user->rsaddr,"193.136.138.142",sizeof(user->rsaddr)-1);
  strncpy(user->rsport,"59000",sizeof(user->rsport)-1);
  user->tcpsessions = 1;
  user->bestpops = 1;
  user->tsecs = 5;
  user->display = ON;
  user->detailed_info = OFF;
  user->synopse = OFF;
}

int read_args(int argc, char **argv, User *user)
{
  int argcount = 0, n;
  char *ptr = NULL, flag[128] = {'\0'};

  for (argcount = 1; argcount < argc; argcount++)
  {
    if(strcmp(argv[argcount],"-i") == 0)
    {
      argcount++;
      strncpy(user->ipaddr,argv[argcount],sizeof(user->ipaddr)-1);
      //printf("%s\n", user->ipaddr);
    }else if(strcmp(argv[argcount],"-t") == 0)
    {
      argcount++;
      strncpy(user->tport,argv[argcount],sizeof(user->tport)-1);
      //printf("%s\n", user->tport);
    }else if(strcmp(argv[argcount],"-u") == 0)
    {
      argcount++;
      strncpy(user->uport,argv[argcount],sizeof(user->uport)-1);
      //printf("%s\n", user->uport);
    }else if(strcmp(argv[argcount],"-s") == 0)
    {
      argcount++;
      if(argcount < argc)
      {
        ptr = argv[argcount];
        strcpy(flag,"addr");
        str_to_IP_PORT(ptr, user->stream_name, user->rsaddr, user->rsport, flag);
        if(strcmp(flag,"BAD_ID") == 0) return 0;
      }
    }else if(strcmp(argv[argcount],"-p") == 0)
    {
      argcount++;
      user->tcpsessions = atoi(argv[argcount]);
      //printf("%d\n", user->tcpsessions);
    }else if(strcmp(argv[argcount],"-n") == 0)
    {
      argcount++;
      user->bestpops = atoi(argv[argcount]);
    }else if(strcmp(argv[argcount],"-x") == 0)
    {
      argcount++;
      user->tsecs = atoi(argv[argcount]);
    }else if(strcmp(argv[argcount],"-b") == 0)
    {
      user->display = OFF;
    }else if(strcmp(argv[argcount],"-d") == 0)
    {
      user->detailed_info = ON;
    }else if(strcmp(argv[argcount],"-h") == 0)
    {
      user->synopse = ON;
    }else{
      //Default case stands for streamID input
      ptr = argv[argcount];
      strcpy(flag,"streamID");
      //str_to_IP_PORT(ptr, user->stream_name, user->stream_addr, user->stream_port, flag);
      //if(strcmp(flag,"BAD_ID") == 0) return 0;
      if(sscanf(ptr, "%[^:]:%[^:]:%s%n", user->stream_name, user->stream_addr, user->stream_port, &n)==3)
      {
        printf("%s\n", user->stream_name);
        printf("%s\n", user->stream_addr);
        printf("%s\n", user->stream_port);
      }else{
        printf("BAD STREAM ID\n");
        return 0;
      }
    }
  }
  if(strcmp(user->stream_name,"") == 0) return 0; //No stream specified
  else return 1;
}

void msg_in_protocol(char *msg, char *label, User *user)
{
  if(strcmp(label,"WHOISROOT")==0)
  {
    snprintf(msg, 128, "WHOISROOT %s:%s:%s %s:%s\n",
    user->stream_name, user->stream_addr, user->stream_port, user->ipaddr, user->uport);
  }
  if(strcmp(label,"REMOVE")==0)
  {
    snprintf(msg, 128, "REMOVE %s:%s:%s\n",
    user->stream_name, user->stream_addr, user->stream_port);
  }
  if(strcmp(label,"POPREQ")==0)
  {
    snprintf(msg, 128, "POPREQ\n");
  }

}

int handle_RSmessage(char *msg, User *user)
{
  int n = 0;
  char *ptr;
  char msgID[128] = {'\0'};
  char stream_name[128] = {'\0'};
  char ipaddr[128] = {'\0'};
  char port[128] = {'\0'};
  char buffer[128] = {'\0'};

  ptr = msg;
  ptr += str_to_msgID(ptr,msgID);

  if(strcmp(msgID,"URROOT")==0)
  {
    str_to_IP_PORT(ptr,stream_name,ipaddr,port,"streamID");
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
      //strcpy(flag,"BAD_ID");
    }
    user->state = access_server;
    user->fd_udp_serv = serv_udp(user->uport);
    user->fd_tcp_serv = serv_tcp(user->tport);
    user->fd_tcp_mont = reach_tcp(user->stream_addr,user->stream_port);
  }else if(strcmp(msgID,"ROOTIS")==0)
  {
    ptr += str_to_IP_PORT(ptr,stream_name,ipaddr,port,"streamID");
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
      //strcpy(flag,"BAD_ID");
    }
    str_to_IP_PORT(ptr,stream_name,ipaddr,port,"addr");
    strcpy(msg,"POPREQ\n");
    reach_udp(ipaddr,port,msg);
    user->state = waiting;
  }
  return 1;
}

int handle_ASmessage(char *msg, User *user)
{
  int n = 0;
  char *ptr;
  char msgID[128] = {'\0'};
  char stream_name[128] = {'\0'};
  char ipaddr[128] = {'\0'};
  char port[128] = {'\0'};
  char buffer[128] = {'\0'};

  ptr = msg;
  ptr += str_to_msgID(ptr,msgID);

  if(strcmp(msgID,"POPRESP")==0)
  {
    ptr += str_to_IP_PORT(ptr,stream_name,ipaddr,port,"streamID");
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
      //strcpy(flag,"BAD_ID");
    }
    str_to_IP_PORT(ptr,stream_name,ipaddr,port,"addr");
    user->fd_tcp_mont = reach_tcp(ipaddr,port);
    user->state = waiting;
  }else if(strcmp(msgID,"POPREQ")==0)
  {
    //BATOTA, ta a mandar o seu POP
    snprintf(msg, 128, "POPRESP %s:%s:%s %s:%s\n",
    user->stream_name, user->stream_addr, user->stream_port, user->ipaddr, user->tport);
    return 1;
  }else return 0;

  return 1;
}
