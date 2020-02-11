#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UNLIMIT
#define MAXARRAY 1000 /* this number, if too large, will cause a seg. fault!! */

struct myStringStruct {
  char qstring[128];
};

int compare(const void *elem1, const void *elem2)
{
  int result;
  
  result = strcmp((*((struct myStringStruct *)elem1)).qstring, (*((struct myStringStruct *)elem2)).qstring);

  return (result < 0) ? 1 : ((result == 0) ? 0 : -1);
}

extern unsigned char input_small_dat[];
extern unsigned int input_small_dat_len;

int
main(int argc, char *argv[]) {
  struct myStringStruct array[MAXARRAY];
  FILE *fp;
  int i,count=0;

  int in_idx;
  int string_idx = 0;
  for (in_idx = 0; in_idx < input_small_dat_len; in_idx++) {
    if (count >= MAXARRAY) {
        break;
    }

    if(input_small_dat[in_idx] == '\n') {
        array[count].qstring[string_idx] = '\0';
        string_idx = 0;
        count += 1;
    } else {
        array[count].qstring[string_idx] = input_small_dat[in_idx];
        string_idx += 1;
    }
  }
  array[count].qstring[string_idx] = '\0';

  printf("\nSorting %d elements.\n\n",count);
  qsort(array,count,sizeof(struct myStringStruct),compare);
  
  for(i=0;i<count;i++)
    printf("%s\n", array[i].qstring);
  return 0;
}
