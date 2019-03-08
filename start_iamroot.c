#include <stdio.h>
#include "functions.h"
#include "udp.h"
#include "tcp.h"

int main(int argc, char **argv)
{
  int maxfd, counter;
  char buffer[128] = {'\0'}, msg[128] = {'\0'};
  fd_set rfds;
  struct sockaddr_in addr;
  int n,nread,nw;
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
    strcpy(msg,"DUMP\n");
    reach_udp(user->rsaddr,user->rsport,msg);
    return 0;
  }

  //Programa começa fora da àrvore
  user->state = out;

  while(1)
  {
    //Reinicia os vetor de file descriptors
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO,&rfds); maxfd = 0; //Sets stdin file descriptor

    //Procedimento para aceder ao stream
    if(user->state == out)
    {
      //Pergunta ao servidor de raizes acerca de um servidor de acesso
      msg_in_protocol(msg,"WHOISROOT",user);
      reach_udp(user->rsaddr,user->rsport,msg); //Sends and waits for answer

      //Process response from root server
      if(handle_RSmessage(msg, user) == 0)
      {
        printf("Could not process response from root_server\n");
        return 0;
      }

      //There was an access server for the stream
      if(user->state == waiting) //waiting state until 'welcome' is recieved
      {
        //Process response from access server
        if(handle_ASmessage(msg, user) == 0)
        {
          printf("Could not process response from access_server\n");
          return 0;
        }
      }
    }
    //Set UDP server if program is the access server for the stream
    if(user->state == access_server)
    {
      FD_SET(user->fd_udp_serv,&rfds);maxfd=max(maxfd,user->fd_udp_serv);  //Servidor de Acesso
    }
    //Set TCP server if program is in the tree
    if((user->state == in)||(user->state == access_server))
    {
      FD_SET(user->fd_tcp_serv,&rfds);maxfd=max(maxfd,user->fd_tcp_serv);
    }
    //Set TCP connection with program a montante
    if(user->state != out)
    {
      FD_SET(user->fd_tcp_mont,&rfds);maxfd=max(maxfd,user->fd_tcp_mont);
    }
    //Select from set file descriptors
    counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
    if(counter<=0){printf("Counter <= 0\n");exit(1);}


    if(FD_ISSET(user->fd_udp_serv,&rfds) && user->state == access_server) //Servidor de Acesso
    {
      addrlen=sizeof(addr);
      nread=recvfrom(user->fd_udp_serv,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
      if(nread==-1)/*error*/exit(1);

      write(1,buffer,nread);
      handle_ASmessage(buffer,user);

      nread = strlen(buffer);
      n=sendto(user->fd_udp_serv,buffer,nread,0,(struct sockaddr*)&addr,addrlen);
      if(n==-1)/*error*/exit(1);
    }
    if(FD_ISSET(user->fd_tcp_serv,&rfds) && user->state != waiting) //Clientes/Abaixo
    {
      if((newfd=accept(user->fd_tcp_serv,(struct sockaddr*)&addr,&addrlen))==-1)
      {
        printf("error: accept\n");
        exit(1);
      }
      ptr = strcpy(buffer,"I SEE YOU\n");
      n = strlen("I SEE YOU\n");
      while(n>0)
      {
        if((nw=write(newfd,ptr,n))<=0){printf("error: write\n"); exit(1);}
        n-=nw; ptr+=nw;
      }
    }
    if(FD_ISSET(user->fd_tcp_mont,&rfds))     //Fonte/Acima
    {
      if((n=read(user->fd_tcp_mont,buffer,128))!=0)
      {
        if(n==-1){printf("error: read\n"); exit(1);}
        ptr=&buffer[0];
        while(n>0)
        {
          if((nw=write(1,ptr,n))<=0){printf("error: write\n"); exit(1);}
          n-=nw; ptr+=nw;
        }
      }else{
        //This close crashes select because it tries to set it again
        close(user->fd_tcp_mont);
        user->state = out;
      }
    }
    if(FD_ISSET(STDIN_FILENO,&rfds))   //STDIN
    {
      fgets(buffer, sizeof(buffer), stdin);
      
      handle_STDINmessage(buffer,user);
      nread = strlen(buffer);
      
      
     
    }


  }//while(1)

}

//Pedro's PC
//./iamroot grupo44:193.136.138.142:58001 -i 192.168.1.67 -u 58001 -t 58001
