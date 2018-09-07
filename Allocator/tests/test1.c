// Basic test

#include <stdio.h>
#include <malloc.h>
#include <string.h>
int main(){

  char* data = (char*)malloc(4096);
  char* data2 = (char*)malloc(4095);
  free(data);
  free(data2);
  return 0;
}
