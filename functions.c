#include "functions.h"
#include "udp.h"
#include "tcp.h"

//String handling
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

int str_to_IP_PORT(char *ptr, char *ipaddr, char *port)
{
  int n = 0, ncount = 0;

  if(sscanf(ptr, "%[^:]:%s%n", ipaddr, port, &n)!=2)
  {
    printf("Unable to read ip:port\n");
    return 0;
  }
  ncount += n;
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
  ncount += n;
  ncount++;

  return ncount;
}

//Setup do programa
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

int read_args(int argc, char **argv, User *user) //Precisa defesa contra opção sem nada depois
{
  int argcount = 0, n;
  char *ptr = NULL, flag[128] = {'\0'};

  for (argcount = 1; argcount < argc; argcount++)
  {
    if(strcmp(argv[argcount],"-i") == 0)
    {
      argcount++;
      strncpy(user->ipaddr,argv[argcount],sizeof(user->ipaddr)-1);
    }else if(strcmp(argv[argcount],"-t") == 0)
    {
      argcount++;
      strncpy(user->tport,argv[argcount],sizeof(user->tport)-1);
    }else if(strcmp(argv[argcount],"-u") == 0)
    {
      argcount++;
      strncpy(user->uport,argv[argcount],sizeof(user->uport)-1);
    }else if(strcmp(argv[argcount],"-s") == 0)
    {
      argcount++;
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
      user->tcpsessions = atoi(argv[argcount]);
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
      str_to_streamID(ptr, user->stream_name, user->stream_addr, user->stream_port);
    }
  }
  if(strcmp(user->stream_name,"") == 0) return 0; //No stream specified

  user->fd_clients = (int *)malloc(sizeof(int)*(user->tcpsessions)); //Array com tcpsessions file descriptors para os jusantes
  memset(user->fd_clients, 0, sizeof(int)*user->tcpsessions);

  user->myClients = malloc(sizeof(char*)*(user->tcpsessions));
  for (int n = 0; n < user->tcpsessions; n++)
  {
    user->myClients[n] = malloc(128*sizeof(char));
    memset(user->myClients[n],'\0',128);
  }


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

int handle_RSmessage(char *msg, User *user)
{

  char *ptr;
  char msgID[128] = {'\0'};
  char stream_name[128] = {'\0'};
  char ipaddr[128] = {'\0'};
  char port[128] = {'\0'};

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
    user->state = access_server;
    user->fd_udp_serv = serv_udp(user->uport);
    user->fd_tcp_mont = reach_tcp(user->stream_addr,user->stream_port);
    if(user->fd_tcp_serv == 0){user->fd_tcp_serv = serv_tcp(user->tport);}
  }else if(strcmp(msgID,"ROOTIS")==0)
  {
    ptr += str_to_streamID(ptr, stream_name, ipaddr, port);
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
    }

    str_to_IP_PORT(ptr, ipaddr, port);
    strcpy(msg,"POPREQ\n");
    reach_udp(ipaddr,port,msg);
    user->state = waiting;
  }
  return 1;
}

int handle_ASmessage(char *msg, User *user)
{
  char *ptr;
  char msgID[128] = {'\0'};
  char stream_name[128] = {'\0'};
  char ipaddr[128] = {'\0'};
  char port[128] = {'\0'};

  ptr = msg;
  ptr += str_to_msgID(ptr,msgID);

  if(strcmp(msgID,"POPRESP")==0)
  {
    ptr += str_to_streamID(ptr, stream_name, ipaddr, port);
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
    }
    str_to_IP_PORT(ptr, ipaddr, port);

    user->fd_tcp_mont = reach_tcp(ipaddr,port);
    user->state = waiting; //Ainda não defende contra IPs fantasma
  }else if(strcmp(msgID,"POPREQ")==0)
  {
    //BATOTA, ta a mandar o seu POP
    snprintf(msg, 128, "POPRESP %s:%s:%s %s:%s\n",
    user->stream_name, user->stream_addr, user->stream_port, user->ipaddr, user->tport);
  }else{printf("AS Message not in protocol\n"); return 0;}

  return 1;
}

int handle_STDINmessage(char *msg, User *user)
{
  int n = 0;
	char buffer[128] = {'\0'};

	strcpy(buffer,msg);
	if(strcmp(buffer,"streams\n")==0)
  {
		strcpy(msg,"DUMP\n");
		reach_udp(user->rsaddr,user->rsport,msg);
  }
	if(strcmp(buffer,"status\n")==0)
	{
		printf("Stream name: %s /n",user->stream_name); //identificação do stream;
		//indicação se o stream está interrompido;
		//Fazer depois, possivelmente adicionar  ao user


		if(user->state==access_server)
		{
			printf("You are the root /n");//indicação se a aplicação é raiz da árvore de escoamento;
			printf("IP and port of Acess server:%s : %s/n",user->ipaddr,user->uport);//endereço IP e porto UDP do servidor de acesso, se for raiz;
		}
		if(user->state==in)
		{
			printf("IP and port of Root a montante:%s : %s/n",user->stream_addr,user->stream_port);//endereço IP e porto TCP do ponto de acesso onde está ligado (a montante), se não for raiz;
			printf("IP and port of Acess server:%s : %s/n",user->ipaddr,user->tport);//endereço IP e porto TCP do ponto de acesso disponibilizado;
			printf("Max number of clients:%d/n",user->tcpsessions);//mero de sessões TCP suportadas a jusante e indicação de quantas se encontram ocupadas;
		}
					//endereço IP e porto TCP dos pontos de acesso dos pares imediatamente a jusante.

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
		//tree query

	}
	if(strcmp(buffer,"exit\n")==0)
	{
    if(user->state == access_server)
    {
      msg_in_protocol(msg,"REMOVE",user);
  		send_udp(user->rsaddr,user->rsport,msg);
    }
    while(n < user->tcpsessions)
    {
      if(user->fd_clients[n] != 0)
      {
        close(user->fd_clients[n]);
        free(user->myClients[n]);
      }
      n++;
    }
    free(user->fd_clients);
    free(user->myClients);
		close(user->fd_udp_serv);
		close(user->fd_tcp_serv);
		close(user->fd_tcp_mont);
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
  reach_udp(user->rsaddr,user->rsport,msg);

  //Processa resposta do root server
  if(handle_RSmessage(msg, user) == 0)
  {
    printf("Could not process response from root_server\n");
    return 0;
  }

  //Há um servidor de acesso para o stream escolhido?
  if(user->state == waiting) //Estado 'waiting' enquanto não recebe um "WELCOME" a confirmar adesão à árvore
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
