/* For copyright information, see olden_v1.0/COPYRIGHT */

#ifdef SS_PLAIN
#include "ssplain.h"
#endif SS_PLAIN

int flag;

int mylog(int num)
{
  int j=0,k=1;
  
  while(k<num) { k*=2; j++; }
  return j;
} 

int dealwithargs(int argc, char *argv[])
{
  int num;

  if (argc > 2)
    flag = atoi(argv[2]);
  else 
    flag = 0;

  if (argc > 1)
    num = atoi(argv[1]);
  else
    num = 15;

  return num;
}
