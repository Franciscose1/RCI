#include <stdio.h>
#include "udp.h"


int main(int argc, char **argv)
{
  int argcount=0;
  //No stream specified -> List available streams
  if(argc < 2)
  {
    list_streams();
  }else{
    //goes through all user passed arguments
    for(argcount=2;argcount<=argc;argcount++)
    {

      printf("Hello World\n");
      printf("This is me\n");
      printf("Pedro\n");
    }
  }
}
