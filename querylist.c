#include "querylist.h"

QueryList* create_query(char *queryID, int bestpops)
{
  QueryList *ql = (QueryList *)malloc(sizeof(QueryList));
  strcpy(ql->queryID,queryID);
  ql->bestpops = bestpops;
  ql->next = NULL;
  return ql;
}

void add_query(QueryList *ql, char *queryID, int bestpops)
{
  QueryList *curr;

  curr = ql;
  while(curr->next != NULL)
  {
    curr = curr->next;
  }

  curr->next = create_query(queryID, bestpops);
  ql->bestpops++;
}

int update_query(QueryList *ql, char *queryID)
{
  int bestpops = 0;
  QueryList *curr, *aux;

  curr = ql;
  while(curr->next != NULL)
  {
    aux = curr->next;
    if(strcmp(aux->queryID,queryID) == 0)
    {
      aux->bestpops--;
      bestpops = aux->bestpops;
      if(aux->bestpops <= 0)
      {
        curr->next = aux->next;
        free(aux);
        break;
      }
    }
    curr = aux;
  }
  return bestpops;
}

int check4query(QueryList *ql, char *queryID)
{
  int query_found = 0;
  QueryList *curr, *aux;

  curr = ql;
  while(curr->next != NULL)
  {
    aux = curr->next;
    if(strcmp(aux->queryID,queryID) == 0)
    {
      query_found = 1;
    }
    curr = aux;
  }
  return query_found;
}

void remove_query(QueryList *ql, char *queryID)
{
  QueryList *curr, *aux;

  curr = ql;
  while(curr->next != NULL)
  {
    aux = curr->next;
    if(strcmp(aux->queryID,queryID) == 0)
    {
      curr->next = aux->next;
      free(aux);
      break;
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

int Randoms(int lower, int upper)
{
  int num = (rand() % (upper - lower + 1)) + lower;

  return num;
}
