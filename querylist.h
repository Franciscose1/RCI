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

QueryList* create_query(char *, int );
int update_query(QueryList *, char *, int );
void print_querys(QueryList *ql);

#endif
