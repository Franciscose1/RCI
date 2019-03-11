#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct QueryList{
    char queryID[128];
    int bestpops;
    struct QueryList *next;
};

typedef struct QueryList QueryList;

QueryList* create_query(char *queryID, int bestpops)
{
  QueryList *ql = (QueryList *)malloc(sizeof(QueryList));
  strcpy(ql->queryID,queryID);
  ql->bestpops = bestpops;
  ql->next = NULL;
  return ql;
}
void update_query(QueryList *ql, char *queryID, int avails)
{
  QueryList *curr, *aux;

  curr = ql;
  while(curr->next != NULL)
  {
    aux = curr->next;
    if(strcmp(aux->queryID,queryID) == 0)
    {
      aux->bestpops -= avails;
      if(aux->bestpops <= 0)
      {
        curr->next = aux->next;
        free(aux);
        break;
      }
    }
    curr = aux;
  }
}
void print_querys(QueryList *ql)
{
  QueryList *curr;

  curr = ql;
  while(curr != NULL)
  {
    printf("ID:%s bestpops:%d\n", curr->queryID, curr->bestpops);
    curr = curr->next;
  }
}

int main(int argc, char const *argv[])
{
  char buffer[128];
  strcpy(buffer,argv[1]);
  QueryList *ql = create_query(buffer,3);
  strcpy(buffer,argv[2]);
  ql->next = create_query(buffer,3);
  strcpy(buffer,argv[3]);
  ql->next->next = create_query(buffer,3);
  printf("BEFORE UPDATE\n");
  print_querys(ql);
  strcpy(buffer,argv[2]);
  update_query(ql,buffer,2);
  printf("AFTER UPDATE\n");
  print_querys(ql);

  return 0;
}
