#include <stdio.h>
#include "functions.h"
#include "udp.h"
#include "tcp.h"

int main(int argc, char **argv)
{
  int maxfd, counter;
  char buffer[128] = {'\0'};
  fd_set rfds;
  struct sockaddr_in addr;
  int n,nw,i;
  unsigned int addrlen;
  int newfd;
  char *ptr;

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
    return 0;
  }

  //Programa começa fora da àrvore
  user->state = out;

  while(1)
  {

    //Reinicia o buffer
    memset(buffer,'\0',sizeof(buffer));

    //Reinicia os vetor de file descriptors
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO,&rfds); maxfd = 0; //Sets stdin file descriptor

    //Procedimento para aceder ao stream
    if(user->state == out)
    {
      if(join_tree(user) == 0)
      {
        printf("Unable to reach stream service tree\n");
        exit(1);
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
    //Set TCP server if program is in the tree
    if((user->state == in)||(user->state == access_server))
    {
      FD_SET(user->fd_tcp_serv,&rfds);maxfd=max(maxfd,user->fd_tcp_serv);
      for(i=0;i<user->tcpsessions;i++)
      {
        FD_SET(user->fd_clients[i],&rfds);maxfd=max(maxfd,user->fd_clients[i]);
      }

    }


    //Select from set file descriptors
    counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
    if(counter<=0){printf("Counter <= 0\n");exit(1);}


    if(FD_ISSET(user->fd_udp_serv,&rfds) && user->state == access_server) //Servidor de Acesso
    {
      recieveNsend_udp(user->fd_udp_serv, buffer, user);
    }
    if(FD_ISSET(user->fd_tcp_serv,&rfds) && user->state != waiting) //Clientes/Abaixo
    {
      if((newfd=accept(user->fd_tcp_serv,(struct sockaddr*)&addr,&addrlen))==-1)
      {
        printf("error: accept\n");
        exit(1);
      }
      for(n=0; n < user->tcpsessions; n++) //Guarda descritor para comunicar com jusante caso haja espaço
      {
        if(user->fd_clients[n] == 0)
        {
          user->fd_clients[n] = newfd;
          //Send WELCOME
          msg_in_protocol(buffer,"WELCOME",user);
          n = strlen(buffer);
          if(send_tcp(buffer,newfd,n) == 0) return 0;
          break;
        }
      }
      if(n == user->tcpsessions)
      {
        //Send Redirect
        msg_in_protocol(buffer,"REDIRECT",user);
        n = strlen(buffer);
        if(send_tcp(buffer,newfd,n) == 0) return 0;
        close(newfd);
      }
    }
    if(FD_ISSET(user->fd_tcp_mont,&rfds) && user->state != out)     //Fonte/Acima
    {
      if((n=read(user->fd_tcp_mont,buffer,128))!=0)
      {
        printf("%s\n", buffer);
        if(n==-1){printf("error: read\n"); exit(1);}
        if(handle_PEERmessage(buffer,user) == 0){printf("Unable to process PEER message\n"); return 0;}
      }else{
        close(user->fd_tcp_mont);
        user->state = out;
      }
    }
    for(i=0;i<user->tcpsessions;i++)
    {
      if(FD_ISSET(user->fd_clients[i],&rfds) && user->fd_clients[i] != 0)
      {
        if((n=read(user->fd_clients[i],buffer,128))!=0)
        {
          printf("%s\n", buffer);
          if(n==-1){printf("error: read\n"); exit(1);}
          if(handle_PEERmessage(buffer,user) == 0){printf("Unable to process PEER message\n"); return 0;}
        }else{
          printf("Client left?\n");
          close(user->fd_clients[i]);
          user->fd_clients[i] = 0;
        }
      }
    }
    if(FD_ISSET(STDIN_FILENO,&rfds))   //STDIN
    {
      fgets(buffer, sizeof(buffer), stdin);

      if(handle_STDINmessage(buffer,user)==0)
      {
        close(newfd);
		    exit(2);
		  }
      n = strlen(buffer);
    }
  }//while(1)

}

//Pedro's PC
//./iamroot grupo44:193.136.138.142:58001 -i 192.168.1.67 -u 58001 -t 58001
//@tecnico
//./iamroot grupo44:193.136.138.142:58001 -i 194.210.159.193 -u 58001 -t 58001

//cd /Users/pedroflores/Documents/IST/5Ano2Sem/RCI/ProjectRepository
