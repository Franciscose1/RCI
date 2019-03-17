#ifndef UDP
#define UDP

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"

void reach_udp(char *, char *, char *);
int send_udp(char *, char *, char *);
int recieveNsend_udp(int , char *, User *);
int serv_udp(char *);

#endif
