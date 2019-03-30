#include "functions.h"
#include "udp.h"
#include "tcp.h"

//String handling
int str_to_msgID(char *ptr, char *msgID)
{
  int n = 0, ncount = 0;

  if(sscanf(ptr, "%s%n", msgID, &n)==1)
  {
    ptr += n; /* advance the pointer by the number of characters read */
    ncount += n;
    if ((*ptr != ' ')&&(*ptr != '\n')&&(*ptr != '\0'))
    {
      printf("Incompatible with protocol\n");
      return 0;
    }
    ncount++;
  }else{
    printf("Failed to read msg_ID\n");
    return 0;
  }
  return ncount;
}

int str_to_IP_PORT(char *ptr, char *ipaddr, char *port)
{
  int n = 0, ncount = 0;

  if(sscanf(ptr, "%[^:]:%s%n", ipaddr, port, &n)!=2)
  {
    //printf("Unable to read ip:port\n");
    return 0;
  }
  ptr += n;
  ncount += n;
  if ((*ptr != ' ')&&(*ptr != '\n')&&(*ptr != '\0'))
  {
    printf("Incompatible with protocol\n");
    return 0;
  }
  ncount++;

  return ncount;
}

int str_to_streamID(char *ptr, char *stream_name, char *ipaddr, char *port)
{
  int n = 0, ncount = 0;

  if(sscanf(ptr, "%[^:]:%[^:]:%s%n", stream_name, ipaddr, port, &n)!=3)
  {
    printf("Unable to read sreamID\n");
    return 0;
  }
  ptr += n;
  ncount += n;
  if ((*ptr != ' ')&&(*ptr != '\n')&&(*ptr != '\0'))
  {
    printf("Incompatible with protocol\n");
    return 0;
  }
  ncount++;

  return ncount;
}

//Setup do programa
void USER_init(User *user)
{
  memset(user->stream_name,'\0',sizeof(user->stream_name));
  memset(user->stream_addr,'\0',sizeof(user->stream_addr));
  memset(user->stream_port,'\0',sizeof(user->stream_port));
  memset(user->ipaddr,'\0',sizeof(user->ipaddr));
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
  memset(user->uproot,'\0',sizeof(user->uproot));
}

int read_args(int argc, char **argv, User *user) //Precisa defesa contra opção sem nada depois
{
  int argcount = 0, n;
  char *ptr = NULL, flag[128] = {'\0'};

  for (argcount = 1; argcount < argc; argcount++)
  {
    if(strcmp(argv[argcount],"-i") == 0)
    {
      argcount++;
      if(argv[argcount] == NULL){printf("IP not inserted after '-i'\n"); return 0;}
      strncpy(user->ipaddr,argv[argcount],sizeof(user->ipaddr)-1);
    }else if(strcmp(argv[argcount],"-t") == 0)
    {
      argcount++;
      if(argv[argcount] == NULL){printf("TCP port not inserted after '-t'\n"); return 0;}
      strncpy(user->tport,argv[argcount],sizeof(user->tport)-1);
    }else if(strcmp(argv[argcount],"-u") == 0)
    {
      argcount++;
      if(argv[argcount] == NULL){printf("UDP port not inserted after '-u'\n"); return 0;}
      strncpy(user->uport,argv[argcount],sizeof(user->uport)-1);
    }else if(strcmp(argv[argcount],"-s") == 0)
    {
      argcount++;
      if(argv[argcount] == NULL){printf("Root Server address not inserted after '-s'\n"); return 0;}
      ptr = argv[argcount];
      //IP do root server
      if(sscanf(ptr, "%[^:]%n", user->rsaddr, &n)!=1)
      {
        printf("unable to read root server IP\n");
        return 0;
      }
      //Verificar se foi também especificado o porto do root server
      ptr += n;
      if(*ptr == ':')
      {
        ptr++;
        if(sscanf(ptr, "%s%n", user->rsport, &n)!=1)
        {
          printf("unable to read root server port\n");
          return 0;
        }
      }
    }else if(strcmp(argv[argcount],"-p") == 0)
    {
      argcount++;
      if(argv[argcount] == NULL){printf("tcpsessions not inserted after '-p'\n"); return 0;}
      user->tcpsessions = atoi(argv[argcount]);
    }else if(strcmp(argv[argcount],"-n") == 0)
    {
      argcount++;
      if(argv[argcount] == NULL){printf("bestpops not inserted after '-n'\n"); return 0;}
      user->bestpops = atoi(argv[argcount]);
    }else if(strcmp(argv[argcount],"-x") == 0)
    {
      argcount++;
      if(argv[argcount] == NULL){printf("tsecs not inserted after '-t'\n"); return 0;}
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
      if(str_to_streamID(ptr, user->stream_name, user->stream_addr, user->stream_port) == 0)
      {printf("Unable to read stream_ID\n");return 0;}
    }
  }
  if(strcmp(user->stream_name,"") == 0){printf("No stream specified\n");return 0;} //No stream specified
  if(strcmp(user->ipaddr,"") == 0){printf("No IP specified for this root\n");return 0;} //No IP specified

  user->fd_clients = (int *)malloc(sizeof(int)*(user->tcpsessions)); //Array com tcpsessions file descriptors para os jusantes
  memset(user->fd_clients, 0, sizeof(int)*user->tcpsessions);

  user->myClients = malloc(sizeof(char*)*(user->tcpsessions));
  for (int n = 0; n < user->tcpsessions; n++)
  {
    user->myClients[n] = malloc(128*sizeof(char));
    memset(user->myClients[n],'\0',128);
  }

  user->POPlist = malloc(sizeof(char*)*(user->bestpops));
  for (int n = 0; n < user->bestpops; n++)
  {
    user->POPlist[n] = malloc(128*sizeof(char));
    snprintf(user->POPlist[n],128,"%s:%s",user->ipaddr,user->tport);
  }

  user->ql = create_query("ListHead",1);

  return 1;
}

//Implementação do protocolo de comunicação
void msg_in_protocol(char *msg, char *label, User *user)
{
  if(strcmp(label,"WHOISROOT")==0)
  {
    snprintf(msg, 128, "WHOISROOT %s:%s:%s %s:%s\n",
    user->stream_name, user->stream_addr, user->stream_port, user->ipaddr, user->uport);
    return;
  }
  if(strcmp(label,"REMOVE")==0)
  {
    snprintf(msg, 128, "REMOVE %s:%s:%s\n",
    user->stream_name, user->stream_addr, user->stream_port);
    return;
  }
  if(strcmp(label,"POPREQ")==0)
  {
    snprintf(msg, 128, "POPREQ\n");
    return;
  }
  if(strcmp(label,"WELCOME")==0)
  {
    snprintf(msg, 128, "WE %s:%s:%s\n", user->stream_name, user->stream_addr, user->stream_port);
    return;
  }
  if(strcmp(label,"REDIRECT")==0)
  {
    char ipaddr[128] = {'\0'}, port[128] = {'\0'};
    str_to_IP_PORT(user->myClients[0], ipaddr, port);
    snprintf(msg, 128, "RE %s:%s\n", ipaddr, port);
    return;
  }
  if(strcmp(label,"NEW_POP")==0)
  {
    snprintf(msg, 128, "NP %s:%s\n", user->ipaddr, user->tport);
    return;
  }
}

int handle_RSmessage(char *msg, User *user) //Servidor de Raizes
{
  char *ptr;
  char msgID[128] = {'\0'};
  char stream_name[128] = {'\0'};
  char ipaddr[128] = {'\0'};
  char port[128] = {'\0'};

  if(user->detailed_info == ON) write(1,msg,strlen(msg));

  ptr = msg;
  ptr += str_to_msgID(ptr,msgID);

  if(strcmp(msgID,"URROOT")==0)
  {
    str_to_streamID(ptr, stream_name, ipaddr, port);
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
    }
    user->fd_udp_serv = serv_udp(user->uport);
    user->fd_tcp_mont = reach_tcp(user->stream_addr,user->stream_port);
    if(user->fd_tcp_serv == 0){user->fd_tcp_serv = serv_tcp(user->tport);}
    if(user->fd_tcp_mont == 0){printf("Unable to connect to stream source\n"); return 0;}
    user->state = access_server;
    dissipate("SF\n",user);
  }else if(strcmp(msgID,"ROOTIS")==0)
  {
    ptr += str_to_streamID(ptr, stream_name, ipaddr, port);
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
    }
    if(user->fd_tcp_serv == 0){user->fd_tcp_serv = serv_tcp(user->tport);}
    str_to_IP_PORT(ptr, ipaddr, port);
    strcpy(msg,"POPREQ\n");
    if(reach_udp(ipaddr,port,msg) == 0) return 0;
  }else{
    printf("RS message not processed\n");
    return 0;
  }
  return 1;
}

int handle_ASmessage(char *msg, User *user) //Servidor de Acesso
{
  int ncount;
  char *ptr;
  char msgID[128] = {'\0'};
  char stream_name[128] = {'\0'};
  char ipaddr[128] = {'\0'};
  char port[128] = {'\0'};

  if(user->detailed_info == ON) write(1,msg,strlen(msg));

  ptr = msg; if((ncount = str_to_msgID(ptr,msgID)) == 0) return 0;
  ptr += ncount;

  if(strcmp(msgID,"POPRESP")==0)
  {
    if((ncount = str_to_streamID(ptr, stream_name, ipaddr, port)) == 0) return 0;
    ptr += ncount;
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
    }
    if(str_to_IP_PORT(ptr, ipaddr, port) == 0) return 0;

    //IP e porto fornecidos não correspondem aos da própria root
    if((strcmp(ipaddr,user->ipaddr) != 0) || (strcmp(port,user->tport) != 0))
    {
      user->fd_tcp_mont = reach_tcp(ipaddr,port);
      memset(user->uproot,'\0',128);
      snprintf(user->uproot,128,"%s:%s",ipaddr,port);

      //Connect SUCCESSFULL (reach_tcp returns 0 otherwise)
      if(user->fd_tcp_mont)
        user->state = in;
    }
  }else if(strcmp(msgID,"POPREQ")==0)
  {
    //Raiz tem disponibilidade
    if(available(user))
    {
      snprintf(msg, 128, "POPRESP %s:%s:%s %s:%s\n",
      user->stream_name, user->stream_addr, user->stream_port, user->ipaddr, user->tport);
    //Raiz não tem disponibilidade, fornece IP:PORT da lista de POPs
    }else{
      snprintf(msg, 128, "POPRESP %s:%s:%s %s\n",
      user->stream_name, user->stream_addr, user->stream_port, user->POPlist[Randoms(0,user->bestpops - 1)]);
    }


  }else{printf("AS Message not in protocol\n"); return 0;}

  return 1;
}

int handle_PEERmessage(char *msg, User *user)
{
  int n = 0, ncount;
  char *ptr;
  char msgID[128] = {'\0'};
  char stream_name[128] = {'\0'};
  char ipaddr[128] = {'\0'};
  char port[128] = {'\0'};

  ptr = msg; if((ncount = str_to_msgID(ptr,msgID)) == 0) return 0;
  ptr += ncount;

  if(strcmp(msgID,"WE") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    if(str_to_streamID(ptr, stream_name, ipaddr, port) == 0) return 0;
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
    }
    msg_in_protocol(msg,"NEW_POP",user);
    send_tcp(msg,user->fd_tcp_mont);
    user->state = waiting;
  }else if(strcmp(msgID,"NP") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    for(n=0; n < user->tcpsessions; n++) //Guarda ip e porto de novo cliente a jusante
    {
      if(strcmp(user->myClients[n],"\0") == 0)
      {
        if(str_to_msgID(ptr,stream_name) == 0) return 0; //Usa-se stream_name como buffer para poupar nas variaveis
        strncpy(user->myClients[n],stream_name,127); //127 porque o tamanho da string é 128. "-1" para guerdar espaço para '\0'
        break;
      }
    }
    if(user->state == in || user->state == access_server) dissipate("SF\n",user);
  }else if(strcmp(msgID,"RE") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    close(user->fd_tcp_mont);
    if(str_to_IP_PORT(ptr, ipaddr, port) == 0) return 0;
    user->fd_tcp_mont = reach_tcp(ipaddr,port);
    memset(user->uproot,'\0',128);
    snprintf(user->uproot,128,"%s:%s",ipaddr,port);
  }else if(strcmp(msgID,"PQ") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    if((ncount = str_to_msgID(ptr,stream_name)) == 0) return 0;
    ptr += ncount; //Usa-se stream_name como buffer para poupar nas variaveis
    sscanf(ptr,"%d",&n);

    //Verifica se já existe uma query em espera com o mesmo queryID
    if(check4query(user->ql,stream_name))
    {
      //Remove query antiga da lista para não haver ambiguidade
      remove_query(user->ql,stream_name);
    }
    //Adiciona query a lista de querys que mantem
    add_query(user->ql,stream_name,n);

    if((n = available(user)) > 0)
    {
      //POPREPLY
      snprintf(msg,128,"PR %s %s:%s %d\n", stream_name, user->ipaddr, user->tport, n);
      send_tcp(msg,user->fd_tcp_mont);

      //Atualiza a query decrementando bestpops
      if((n = update_query(user->ql,stream_name)) > 0) //Verifica quantos POPs ainda são precisos
      {
        //Propaga POPQUERY decrementado
        snprintf(msg,128,"PQ %s %d\n", stream_name, n);
        dissipate(msg,user);
      }
    }else{
      //Dissemina POPQUERY sem o alterar
      dissipate(msg,user);
    }
  }else if(strcmp(msgID,"PR") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    if((ncount = str_to_msgID(ptr,stream_name)) == 0) return 0;
    ptr += ncount; //Usa-se stream_name como buffer para poupar nas variaveis

    //Verifica se espera a query recebida
    if(check4query(user->ql,stream_name))
    {
      //Atualiza query com bestpops decrementado
      n = update_query(user->ql,stream_name);
      if(user->state == access_server)
      {
        //Guarda POP na lista de POPs a fornecer a novas roots
        str_to_IP_PORT(ptr,ipaddr,port);
        memset(user->POPlist[n],'\0',128);
        snprintf(user->POPlist[n],128,"%s:%s",ipaddr,port);
      }else if(user->state == in){
        //Reencaminha a montante o POPreply
        send_tcp(msg,user->fd_tcp_mont);
      }
    }else{
      printf("PR not expected\n");
    }
    print_querys(user->ql);
  }else if(strcmp(msgID,"SF") == 0)
  {
    if(user->state == waiting)
    {
      if(user->detailed_info == ON) write(1,msg,strlen(msg));
      user->state = in;
      dissipate("SF\n",user);
    }
  }else if(strcmp(msgID,"BS") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    user->state = waiting;
    dissipate("BS\n",user);
  }else if(strcmp(msgID,"DA") == 0)
  {
    if(user->display == ON) write(1,msg,strlen(msg));
    sscanf(ptr,"%d",&n);
    n++;
    snprintf(msg,128,"%s %d\n",msgID,n);
    dissipate(msg,user);
  }else if(strcmp(msgID,"POPQUERY") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    n = Randoms(user->bestpops,32000); //Give random queryID
    snprintf(msg, 128, "PQ %04X %d\n",n, user->bestpops);
    snprintf(ipaddr,128,"%04X",n);

    //Verifica se já existe uma query em espera com o mesmo queryID
    if(check4query(user->ql,stream_name))
    {
      //Remove query antiga da lista para não haver ambiguidade
      remove_query(user->ql,stream_name);
    }
    add_query(user->ql,ipaddr,user->bestpops);
    dissipate(msg,user);
  }else if(strcmp(msgID,"LIST") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    for(int i = 0; i < user->bestpops; i++)
    {
      printf("%s\n", user->POPlist[i]);
    }
  }else if(strcmp(msgID,"TQ") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    str_to_IP_PORT(ptr,ipaddr,port);
    if(strcmp(ipaddr,user->ipaddr) == 0 && strcmp(port,user->tport) == 0)
    {
      ptr = stream_name;
      ncount = snprintf(ptr,128,"TR %s:%s %d\n",ipaddr,port,user->tcpsessions);
      for(int i = 0; i < user->tcpsessions; i++)
      {
        if(user->fd_clients[i] == 0)
          break;

        ptr += ncount;
        ncount = snprintf(ptr,128,"%s ",user->myClients[i]);
      }
      ptr += ncount-1;
      snprintf(ptr,128,"\n");
      if(send_tcp(stream_name,user->fd_tcp_mont) == 0)
      {
        close(user->fd_tcp_mont);
        user->state = out;
        dissipate("BS\n",user);
      }
    }else{
      dissipate(msg,user);
    }

  }else if(strcmp(msgID,"TREEQUERY") == 0)
  {
    if(user->detailed_info == ON) write(1,msg,strlen(msg));
    for(int i = 0; i < user->tcpsessions; i++)
    {
      if(user->fd_clients[i] != 0)
      {
        snprintf(stream_name,128,"TQ %s\n", user->myClients[i]);
        if(send_tcp(stream_name,user->fd_clients[i]) == 0)
        {
          printf("Client left?\n");
          close(user->fd_clients[i]);
          user->fd_clients[i] = 0;
          memset(user->myClients[i],'\0',128);
        }
      }
    }
  }else if(strcmp(msgID,"TR") == 0)
  {
    if(user->state == access_server)
    {
      if(user->detailed_info == ON) write(1,msg,strlen(msg));
      ncount = str_to_IP_PORT(ptr,ipaddr,port);
      ptr += ncount;
      sscanf(ptr,"%d",&n);
      ptr+=2;
      printf("%s:%s (%d",ipaddr,port,n);
      while((ncount = str_to_IP_PORT(ptr,ipaddr,port)))
      {

        ptr += ncount;
        printf(" %s:%s",ipaddr,port);
        snprintf(stream_name,128,"TQ %s:%s\n",ipaddr,port);
        dissipate(stream_name,user);
      }
      printf(")\n");
    }else if(user->state == in){
      if(user->detailed_info == ON) write(1,msg,strlen(msg));
      if(send_tcp(msg,user->fd_tcp_mont) == 0)
      {
        close(user->fd_tcp_mont);
        user->state = out;
      }
    }
  }else if(user->state == access_server)
  {
    n = strlen(msg);
    snprintf(stream_name,128,"DA %04X %s\n",n,msg);
    if(user->display == ON) write(1,msg,strlen(msg));
    dissipate(stream_name,user);
  }
  return 1;
}

int handle_STDINmessage(char *msg, User *user) //STDIN
{
	char buffer[128] = {'\0'};

	strcpy(buffer,msg);
	if(strcmp(buffer,"streams\n")==0)
  {
		strcpy(msg,"DUMP\n");
		if(reach_udp(user->rsaddr,user->rsport,msg) == 0) return 0;
    write(1,msg,strlen(msg));
  }
	if(strcmp(buffer,"status\n")==0)
	{
		printf("Stream name: %s \n",user->stream_name); //identificação do stream;

    if(user->state == in) //Aplicação está ligada à árvore
    {
      //printf("Stream is flowing\n");
      printf("I am not the root\n");
      printf("IP and port of Root a montante: %s\n",user->uproot);//endereço IP e porto TCP do ponto de acesso a montante
      printf("My clients:\n");
      for(int i = 0; i < user->tcpsessions; i++)
      {
        printf("%s\n", user->myClients[i]);
      }

    }else if(user->state == access_server) //Indicação se a aplicação é a raiz da árvore de escoamento
    {
      //printf("Stream is flowing\n");
      printf("I am Groot!\n");
      printf("My access server address: %s:%s\n", user->ipaddr, user->uport);
      printf("My clients:\n");
      for(int i = 0; i < user->tcpsessions; i++)
      {
        printf("%s\n", user->myClients[i]);
      }
    }else if(user->state == out) //Fora da árvore
    {
      printf("I am not the root\n");
      printf("Out of stream tree\n");
    }
	}
	if(strcmp(buffer,"display on\n")==0)
		user->display = ON;
	if(strcmp(buffer,"display off\n")==0)
		user->display = OFF;
	if(strcmp(buffer,"debug on\n")==0)
		user->detailed_info = ON;
	if(strcmp(buffer,"debug off\n")==0)
		user->detailed_info = OFF;
	if(strcmp(buffer,"tree\n")==0)
	{
    if(user->state == access_server)
    {
      strcpy(buffer,"TREEQUERY\n");
      handle_PEERmessage(buffer,user);
    }else{
      printf("I am not the root :(\n");
    }
	}
	if(strcmp(buffer,"exit\n")==0)
	{
    clean_exit(user);
    printf("EXIT SUCCESSFULL\n");
		return(0);
	}

return 1;
}

//Mecanismo de adesão à árvore
int join_tree(User *user)
{
  char msg[128] = {'\0'};

  //Pergunta ao servidor de raizes acerca de um servidor de acesso
  msg_in_protocol(msg,"WHOISROOT",user);

  if(reach_udp(user->rsaddr,user->rsport,msg) == 0) return 0;

  //Processa resposta do root server
  if(handle_RSmessage(msg, user) == 0)
  {
    printf("Could not process response from root_server\n");
    return 0;
  }

  //Há um servidor de acesso para o stream escolhido?
  if(user->state == out)
  {
    //Processa resposta do servidor de acesso
    if(handle_ASmessage(msg, user) == 0)
    {
      printf("Could not process response from access_server\n");
      return 0;
    }
  }

  return 1;
}

//Verifica disponibilidade
int available(User *user)
{
  int i,n = 0;

  for(i = 0; i < user->tcpsessions; i++)
  {
    if(user->fd_clients[i] == 0)
      n++;
  }
  return n;
}

//Fecha sockets e liberta memória alocada
void clean_exit(User *user)
{
  int n;
  char msg[128] = {'\0'};

  if(user->state == access_server)
  {
    msg_in_protocol(msg,"REMOVE",user);
    send_udp(user->rsaddr,user->rsport,msg);
    close(user->fd_udp_serv);
  }
  for(n = 0; n < user->tcpsessions; n++)
  {
    free(user->myClients[n]);

    //Previne fechar STDIN_FILENO
    if(user->fd_clients[n] != 0)
    {
      close(user->fd_clients[n]);
    }
  }
  for(n = 0; n < user->bestpops; n++)
  {
    free(user->POPlist[n]);
  }
  free(user->fd_clients);
  free(user->myClients);
  free(user->POPlist);
  close(user->fd_tcp_serv);
  close(user->fd_tcp_mont);

  return;
}

//Apresenta sinopse dos comandos
void synopse()
{
  printf("iamroot\n[<streamID>]\n[-i <ipaddr>]\n[-t <tport>]\n[-u <uport>]\n[-s <rsaddr>[<:rsport>]]\n[-p <tcpsessions>]\n[-n <bestpops>]\n[-x <tsecs>]\n[-b <display stream>][-d <detailed info>]\n[-h <this synopse>]\n");
}
