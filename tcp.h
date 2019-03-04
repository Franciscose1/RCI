#ifndef TCP
#define TCP

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"

void reach_tcp(char *, char *, int *);
int serv_tcp(char *);

#endif
