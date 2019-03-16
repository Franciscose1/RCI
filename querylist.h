#ifndef QUERYLIST
#define QUERYLIST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct QueryList{
    char queryID[128];
    int bestpops;
    struct QueryList *next;
};

typedef struct QueryList QueryList;

QueryList* create_query(char *, int);
void add_query(QueryList *, char *, int);
int update_query(QueryList *, char *);
int check4query(QueryList *, char *);
void print_querys(QueryList *);
int Randoms(int , int);

#endif
