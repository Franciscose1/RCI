#include <stdio.h>
#include "functions.h"
#include "udp.h"
#include "tcp.h"

int main(int argc, char **argv)
{
  int maxfd, counter, fd_udp_serv, fd_tcp_mont, fd_tcp_serv;
  char buffer[128] = {'\0'}, msg[128] = {'\0'}, resp[128] = {'\0'};
  //enum {access_server,out,in} state;
  fd_set rfds;
  struct sockaddr_in addr;
  int n,nread;
  unsigned int addrlen;

  int newfd;

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

      if(handle_RSmessage(msg, user, &fd_udp_serv) == 0)
      {
        printf("Could not process response from root_server\n");
        return 0;
      }
      if(user->state != access_server)
      {
        if(handle_ASmessage(msg, user, &fd_tcp_mont) == 0)
        {
          printf("Could not process response from access_server\n");
          return 0;
        }
      }else{
        fd_tcp_serv = serv_tcp(user->tport);
      }
    }

    if(user->state == access_server)
    {
      FD_SET(fd_udp_serv,&rfds);maxfd=max(maxfd,fd_udp_serv);
    }
    if(user->state != out)
    {
      FD_SET(fd_tcp_serv,&rfds);maxfd=max(maxfd,fd_tcp_serv);
    }

    counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
    if(counter<=0){printf("Counter <= 0\n");exit(1);}

    if(FD_ISSET(fd_udp_serv,&rfds) && user->state == access_server)
    {
      addrlen=sizeof(addr);
      nread=recvfrom(fd_udp_serv,buffer,128,0,(struct sockaddr*)&addr,&addrlen);
      if(nread==-1)/*error*/exit(1);
      write(1,buffer,nread);
      n=sendto(fd_udp_serv,buffer,nread,0,(struct sockaddr*)&addr,addrlen);
      if(n==-1)/*error*/exit(1);
    }
    if(FD_ISSET(STDIN_FILENO,&rfds))
    {
      fgets(buffer, sizeof(buffer), stdin);
      if(strcmp(buffer,"root_out\n")==0)
      {
        printf("EXIT SUCCESSFULL\n");
        close(fd_udp_serv);
        close(fd_tcp_serv);
        exit(1);
      }
    }
    if(FD_ISSET(fd_tcp_serv,&rfds))
    {
      if((newfd=accept(fd_tcp_serv,(struct sockaddr*)&addr,&addrlen))==-1)
      {
        printf("error: accept\n");
        exit(1);
      }
      printf("HURRAY!\n");
    }

  }//while(1)

}
