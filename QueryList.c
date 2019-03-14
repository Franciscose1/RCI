#include "querylist.h"

QueryList* create_query(char *queryID, int bestpops)
{
  QueryList *ql = (QueryList *)malloc(sizeof(QueryList));
  strcpy(ql->queryID,queryID);
  ql->bestpops = bestpops;
  ql->next = NULL;
  return ql;
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
      if(aux->bestpops <= 0)
      {
        curr->next = aux->next;
        ql->bestpops--; //A ListHead guarda o numero de querys
        free(aux);
        break;
      }
    }
    curr = aux;
  }
  return bestpops;
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

/*int main(int argc, char const *argv[])
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
*/
