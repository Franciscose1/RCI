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
  if(strcmp(label,"DA")==0)
  {
    snprintf(msg, 128, "DA %d\n\n");
  }

}

int handle_RSmessage(char *msg, User *user) //Servidor de Raizes
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

int handle_ASmessage(char *msg, User *user) //Servidor de Acesso
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


int handle_PACKETmessage(char *msg, User *user, int *nbytesleft,int totalbytes, int bytesread)
{
	if(bytesread <= totalbytes-nbytesleft){
		memcpy(msg,buffer,nbytes);
	
	
	}


}

int handle_SOURCEmessage(char *msg, User *user)
{
	char buffer[128] = {'\0'};
	if((nbytes=read(user->fd_tcp_mont,buffer,128))!=0)
      {
		  
        if(nbytes==-1){printf("error: read\n"); exit(1);}
		ptr=&buffer[0];
        while(nbytes>0)
        {
          if((nw=write(1,ptr,nbytes))<=0){printf("error: write\n"); exit(1);}
          nbytes-=nw; ptr+=nw;
        }
        
        
        //Transforma em modo protocolo
        char *msg = (char *)malloc(nbytes+7);
		snprintf(msg, 128, "DA %4.4h\n",nbytes);
		memcpy(msg+7,buffer,nbytes);

		for(n=0; n < user->tcpsessions; n++) //Envia para os a montante
			if(user->fd_clients[n] != 0)
				if(send_tcp(buffer,user->fd_clients[n],nbytes+7) == 0) return 0;
		free(msg);	

	}
}
int handle_R2Rmessage(char *msg, User *user) //iamroot to iamroot
{
  int nbytes,nw;
  char *ptr;
  char msgID[128] = {'\0'};
	
  char buffer[128] = {'\0'};

  ptr = msg;
  ptr += str_to_msgID(ptr,msgID);
  
  if(user->state==access_server)//Caso seja o root, recebe da fonte em formato fora do protocolo
	{
		if((nbytes=read(user->fd_tcp_mont,buffer,128))!=0)
      {
        if(nbytes==-1){printf("error: read\n"); exit(1);}
		ptr=&buffer[0];
        while(nbytes>0)
        {
          if((nw=write(1,ptr,nbytes))<=0){printf("error: write\n"); exit(1);}
          nbytes-=nw; ptr+=nw;
        }
        //Transforma em modo protocolo
        char *msg = (char *)malloc(nbytes);
        int total_enviado=0;
		while(total_enviado<count2){
			nbytes = write(clipboard_id, msg+total_enviado, count2-total_enviado);
			if(nbytes==-1){ 
				perror("Write: ");
				return(0);
			}
		
			total_enviado+=nbytes;
		}
	free(msg);	
        
	}
  if(strcmp(msgID,"DA")==0)
  {
		//Ler o tamanho do pacote <nbytes></n><DATA>
		//Recebe o pacote
		char *msg = (char *)malloc(nbytes);
		total_recebido=0;
		while(total_recebido<size_msg2){
			nbytes = read(client_fd, msg+total_recebido, size_msg2-total_recebido);
			if(nbytes==-1){ 
				perror("read: ");
				close(client_fd);
				free(recebe);
				free(msg);
				return(0);
			}			
			total_recebido+=nbytes;
		}
		//Imprime no ecra
		
		//Envia para os que estão abaixo
		int count2=count;												
		void *msg = (void *)malloc(count2);
		memcpy(msg,buf,count2);
	
		int total_enviado=0;
		while(total_enviado<count2){
			nbytes = write(clipboard_id, msg+total_enviado, count2-total_enviado);
			if(nbytes==-1){ 
				perror("Write: ");
				return(0);
			}
		
			total_enviado+=nbytes;
		}
	free(msg);	
		
		
	  free(msg);
	  
  }

  return 1;
}
}

int handle_STDINmessage(char *msg, User *user) //STDIN
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
			//Isto está errado.
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

int handle_PEERmessage(char *msg, User *user)
{
  int n = 0;
  char *ptr;
  char msgID[128] = {'\0'};
  char stream_name[128] = {'\0'};
  char ipaddr[128] = {'\0'};
  char port[128] = {'\0'};

  ptr = msg;
  ptr += str_to_msgID(ptr,msgID);

  if(strcmp(msgID,"WE") == 0)
  {
    str_to_streamID(ptr, stream_name, ipaddr, port);
    if((strcmp(stream_name,user->stream_name)!=0)||(strcmp(ipaddr,user->stream_addr)!=0)||(strcmp(port,user->stream_port)!=0))
    {
      printf("Incompatible stream\n");
      return 0;
    }
    msg_in_protocol(msg,"NEW_POP",user);
    n = strlen(msg);
    send_tcp(msg,user->fd_tcp_mont,n);
    user->state = in;
    if(user->fd_tcp_serv == 0){user->fd_tcp_serv = serv_tcp(user->tport);}
  }
  if(strcmp(msgID,"NP") == 0)
  {
    for(n=0; n < user->tcpsessions; n++) //Guarda ip e porto de novo cliente a jusante
    {
      if(strcmp(user->myClients[n],"\0") == 0)
      {
        str_to_msgID(ptr,stream_name); //Usa-se stream_name como buffer para poupar nas variaveis
        strncpy(user->myClients[n],stream_name,127); //127 porque o tamanho da string é 128. "-1" para guerdar espaço para '\0'
      }
    }
  }
  if(strcmp(msgID,"RE") == 0)
  {
    close(user->fd_tcp_mont);
    str_to_IP_PORT(ptr, ipaddr, port);
    user->fd_tcp_mont = reach_tcp(ipaddr,port);
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
