#include <stdio.h>
#include <signal.h>
#include "functions.h"
#include "udp.h"
#include "tcp.h"
#include <time.h>

#define POPQUERY_TIME 120

int main(int argc, char **argv)
{
  int maxfd, counter, newfd;
  int bytes_avanco=0,packet_left=0,nbytes,packet_total,a,ncount;
  char buffer[128] = {'\0'};
  char packet[1024]={'\0'};
  char msgID[128] = {'\0'};
  fd_set rfds;
  int n,i,tries;
  char *ptr;
  struct timeval tv = {10, 0};

  //Timer para refrescar raiz no servidor de raizes e/ou refrescar lista de POPs
  time_t timer_start = time(NULL);

  //Ignore SIGPIPE signal
  struct sigaction act;
  memset(&act,0,sizeof act);
  act.sa_handler=SIG_IGN;
  if(sigaction(SIGPIPE,&act,NULL)==-1){printf("Unable to handle SIGPIPE\n");exit(1);}

  //Estrutura com a informação sobre o programa e dados passados pelo utilizador
  User *user=(User *)malloc(sizeof(User));

  //Inicializar estrutura com os valores DEFAULT
  USER_init(user);

  //Lê os dados passados pelo utilizador e escreve-os na respetiva estrutura
  if(read_args(argc, argv, user)==0)
  {
    //Apresenta lista de streams disponiveis caso nenhum tenha sido especificado
    strcpy(buffer,"DUMP\n");
    reach_udp(user->rsaddr,user->rsport,buffer);
    if(user->detailed_info == ON) write(1,buffer,strlen(buffer));
    return 0;
  }

  if(user->synopse)
  {
    synopse();
    return 1;
  }

  //Programa começa fora da àrvore
  user->state = out;

  while(1){
	 

    //Reinicia o buffer
    memset(buffer,'\0',sizeof(buffer));

    //Reinicia o timeout
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    //Reinicia os vetor de file descriptors
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO,&rfds); maxfd = 0; //Sets stdin file descriptor

    //Procedimento para aceder ao stream
    if(user->state == out)
    {
      if(join_tree(user) == 0)
      {
        printf("Unable to reach stream service tree\n");
        clean_exit(user);
        exit(1);
      }
      //Melhorar
      if(user->state == out)
      {
        tries++;
        printf("Tried %d times, not able to join stream\n", tries);
        if(tries > 5){clean_exit(user); exit(1);}
        sleep(5);
        continue; //Não conseguiu ligar-se ao IP fornecido, try again
      }
    }
    //Arma descritor para ligação a montante
    if(user->state != out)
    {
      FD_SET(user->fd_tcp_mont,&rfds);maxfd=max(maxfd,user->fd_tcp_mont);
    }
    //Arma descritor para receber mensagens por UDP, caso seja servidor de acesso
    if(user->state == access_server)
    {
      FD_SET(user->fd_udp_serv,&rfds);maxfd=max(maxfd,user->fd_udp_serv);  //Servidor de Acesso
    }
    //Arma descritores para ligações a jusante
    if(user->state != out)
    {
      FD_SET(user->fd_tcp_serv,&rfds);maxfd=max(maxfd,user->fd_tcp_serv);
      for(i=0;i<user->tcpsessions;i++)
      {
        FD_SET(user->fd_clients[i],&rfds);maxfd=max(maxfd,user->fd_clients[i]);
      }
    }

    //Select from set file descriptors
    counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL, &tv);
    if(counter<0){printf("Counter < 0\n");exit(1);}

    if(FD_ISSET(user->fd_udp_serv,&rfds) && user->state == access_server) //Servidor de Acesso
    {
      recieveNsend_udp(user->fd_udp_serv, buffer, user);
    }
    if(FD_ISSET(user->fd_tcp_serv,&rfds) && user->state != out) //Clientes/Jusante
    {
      if(new_connection(user) == 0)
      {
        clean_exit(user);
        exit(1);
      }
    }
    if(FD_ISSET(user->fd_tcp_mont,&rfds) && user->state != out)     //Fonte/Acima
    {
		
		
		
		if(user->state==access_server)//Caso seja o root, recebe da fonte em formato fora do protocolo
		{   
			if(handle_SOURCEmessage(user)==0){
				printf("Problems with the source");
				return 0;
			}
			continue;
		}
		
		
		
		if((n=read(user->fd_tcp_mont,buffer+bytes_avanco,128))!=0)
		{
			bytes_avanco=0;
			//printf("\n 144: n=%d and buffer has: %s\n",n,buffer);				/////////////////////////////
			if(packet_left > 0)
			{
				printf("Temos um pacote incompleto\n");
				nbytes=handle_PACKETmessage(buffer,packet,user,&packet_left,packet_total,n);
				if(nbytes==1) //O pacote ainda não foi todo recebido.
				{
					bytes_avanco=0;
					n=0;
				}
				if(nbytes>1)
				{
					shift_left_buffer(buffer,nbytes,n);
					n=n-nbytes;
				}
			}
				

			while(n >0)
			{
				
				if(sscanf(buffer, "%s%n", msgID, &a)==0)
				{
					printf("Failed to read msg_ID\n");
					return 0;
				}
				//printf("Buffer has:%s",buffer); 		/////////////////////////////
				//printf("String is %s\n",msgID);    ///////////////////////////////////
				if(strcmp(msgID,"DA")==0)
				{	
					if(n<7)
					{
						bytes_avanco=n;
						break;
					}
					//printf("Buffer antes de shift: %s\n",buffer);///////////////////////////////////
					shift_left_buffer(buffer,3,n);
					n=n-3;
					//printf("Buffer depois de shift2: %s\n",buffer);///////////////////////////////////
					sscanf(buffer,"%X",&packet_total);
					packet_left=packet_total;
					//printf("Size of packet:%d\n",packet_total);////////////////////////////////////////
					
					shift_left_buffer(buffer,5,n);
					n=n-5;
					//isto
					//printf("Buffer depois de shift3: %s\n",buffer);///////////////////////////////////
					nbytes=handle_PACKETmessage(buffer,packet,user,&packet_left,packet_total,n);
					if(nbytes==1) //O pacote ainda não foi todo recebido.
					{
						printf("O pacote não foi todo recebido 194\n");
						bytes_avanco=0;
						n=0;
						break;
					}
					if(nbytes>1)
					{
						shift_left_buffer(buffer,nbytes,n);
					//	printf("202:Buffer has:%s\n and nbytes=%d\n",buffer,nbytes);/////////////////////////////
						n=n-nbytes;
					//	printf("204:nis is:%d and packet_left=%d\n",n,packet_left);
														/////////////////////////////
						continue;
					}
				}
				//printf("208:Cheguei a 1\n");										/////////////////////////////
				ptr = buffer; 
				if(find_complete_message(ptr,msgID, &ncount,n) == 0){ 
					bytes_avanco=n;
				//	printf("212:Cheguei a 2 e deixo bytes de avanço: %d\n",bytes_avanco); /////////////////////////////
					break;
				}
				//printf("215\n");						/////////////////////////////
				char aux[128]= {'\0'};
				memcpy(aux,buffer,ncount); //char *aux;	
				shift_left_buffer(buffer,ncount,n);
				n=n-ncount;
				//aux[ncount+1]='\0';                             ///////////////////////////// TESTAR ISTO
			//	printf("220:Aux has %s\n",aux);
				//printf("220\n");					/////////////////////////////
				if(handle_PEERmessage(aux,user) == 0){printf("Unable to process PEER message\n"); clean_exit(user); exit(1);}		
			}
					
		  }else{
			 
			//Montante saiu
			close(user->fd_tcp_mont);
			user->state = out;
			dissipate("BS\n",user);
		  }
    }
    
    
    
    
    
    for(i=0;i<user->tcpsessions;i++)
    {
      if(FD_ISSET(user->fd_clients[i],&rfds) && user->fd_clients[i] != 0)
      {
        //Reinicia o buffer
        memset(buffer,'\0',sizeof(buffer));

        if((n=read(user->fd_clients[i],buffer,128))!=0)
        {
          if(n==-1){printf("error: read\n"); clean_exit(user); exit(1);}
          if(handle_PEERmessage(buffer,user) == 0){printf("Unable to process PEER message\n"); clean_exit(user); exit(1);}
        }else{
          printf("Client left?\n");
          close(user->fd_clients[i]);
          user->fd_clients[i] = 0;
          memset(user->myClients[i],'\0',128);
        }
      }
    }
    if(FD_ISSET(STDIN_FILENO,&rfds))   //STDIN
    {
      fgets(buffer, sizeof(buffer), stdin);

      if(handle_STDINmessage(buffer,user)==0)
      {
		    exit(1);
		  }
      n = strlen(buffer);
    }

    //Timer para o access_server
    if(user->state == access_server)
    {
      if(time(NULL)-timer_start > user->tsecs)
      {
        //Refresca servidor de raizes
        msg_in_protocol(buffer,"WHOISROOT",user);
        reach_udp(user->rsaddr,user->rsport,buffer);
        if(user->detailed_info == ON) write(1,buffer,strlen(buffer));
		
		strcpy(buffer,"POPQUERY\n");
        handle_PEERmessage(buffer,user);
        
        //if(time(NULL)-timer_start > POPQUERY_TIME)
        //{
          //Faz POPQUERY para refrescar POPs
          //strcpy(buffer,"POPQUERY\n");
          //handle_PEERmessage(buffer,user);
        //}

        //Reinicia o timer
        timer_start = time(NULL);
      }
    }else if(user->state == waiting)
    {
      if(counter == 0)
      {
        close(user->fd_tcp_mont);
        user->state = out;
      }
    }
  }//while(1)

}

//Pedro's PC
//./iamroot grupo44:193.136.138.142:58001 -i 192.168.1.67 -u 58001 -t 58001

//My Fonte
//./iamroot grupo44:192.168.1.67:57000 -i 192.168.1.67 -u 58001 -t 58001

// ./iamroot grupo44:192.136.138.142:59000 -i 194.210.157.158 -d -b -x 30
// ./iamroot grupo44:194.210.157.158:57000 -i 194.210.156.33 -d -b -x 30 -u 58001 -t 58001
// nc -l 57000 




//cd /Users/pedroflores/Documents/IST/5Ano2Sem/RCI/ProjectRepository

/* Duvidas
1 - Se o descritor não estiver armado e outro fizer um write, o que é que o
write retorna?
2 - Se der para fazer write, quando armar o descritor ele vai ser logo ativo pelo select?
*/
