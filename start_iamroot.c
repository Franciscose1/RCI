#include <stdio.h>
#include "functions.h"
#include "udp.h"
#include "tcp.h"

int main(int argc, char **argv)
{
  int maxfd, counter;
  char buffer[128] = {'\0'}, msg[128] = {'\0'}, resp[128] = {'\0'};
  //enum {access_server,out,in} state;
  fd_set rfds;
  struct sockaddr_in addr;
  int n,nread,fd_temp;
  unsigned int addrlen;

  int newfd, nw;
  char *ptr;

  //Struct to store user input
  User *user=(User *)malloc(sizeof(User));

  //Initialize structure with some default values
  USER_init(user);

  //Read valid arguments and write them to User struct
  if(read_args(argc, argv, user)==0)
  {
    //List available streams if stream ID is not specified by user
    strcpy(msg,"DUMP\n");
    reach_udp(user->rsaddr,user->rsport,msg);

    return 0;
  }

  //Starts out of the tree
  user->state = out;
  while(1)
  {
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO,&rfds); maxfd = 0;

    if(user->state == out)
    {
      msg_in_protocol(msg,"WHOISROOT",user);
      reach_udp(user->rsaddr,user->rsport,msg);

      if(handle_RSmessage(msg, user) == 0)
      {
        printf("Could not process response from root_server\n");
        return 0;
      }
      if(user->state == waiting)
      {
        if(handle_ASmessage(msg, user) == 0)
        {
          printf("Could not process response from access_server\n");
          return 0;
        }
      }
    }

    if(user->state == access_server)
    {
      FD_SET(user->fd_udp_serv,&rfds);maxfd=max(maxfd,user->fd_udp_serv);
    }
    if((user->state == in)||(user->state == access_server))
    {
      FD_SET(user->fd_tcp_serv,&rfds);maxfd=max(maxfd,user->fd_tcp_serv);
    }
    if(user->state != out)
    {
      FD_SET(user->fd_tcp_mont,&rfds);maxfd=max(maxfd,user->fd_tcp_mont);
    }

    counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
    if(counter<=0){printf("Counter <= 0\n");exit(1);}


    if(FD_ISSET(user->fd_udp_serv,&rfds) && user->state == access_server)
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
    if(FD_ISSET(user->fd_tcp_serv,&rfds) && user->state != waiting)
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
    if(FD_ISSET(user->fd_tcp_mont,&rfds))
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
        close(user->fd_tcp_serv);
        user->state = out;
      }
    }
    if(FD_ISSET(STDIN_FILENO,&rfds))
    {
      fgets(buffer, sizeof(buffer), stdin);
      if(strcmp(buffer,"root_out\n")==0)
      {
        msg_in_protocol(msg,"REMOVE",user);
        send_udp(user->rsaddr,user->rsport,msg);
        printf("EXIT SUCCESSFULL\n");
        close(user->fd_udp_serv);
        close(user->fd_tcp_serv);
        close(user->fd_tcp_mont);
        close(newfd);
        exit(1);
      }
    }

  }//while(1)

}

//./iamroot grupo44:192.168.1.67:57000 -i 192.168.1.67 -u 58001
