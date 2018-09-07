#include <stdio.h>
#include <malloc.h>
#include <pthread.h>

void* getMemory(void* args)
{
      int* data =  (int*)malloc(8);
      int* data1 = (int*)malloc(16);
      int* data2 = (int*)malloc(32);
      int* data3 = (int*)malloc(64);
      int* data4 = (int*)malloc(128);

      free(data);
      free(data1);
      free(data2);
      free(data3);
      free(data4);
      return NULL;
}

int main()
{
  pthread_t tid[100];
  for(int i= 0; i < 100; i++)
  {
       pthread_create(&tid[i], NULL, getMemory, NULL);
       pthread_join(tid[i], NULL);
  }
  pthread_exit(NULL);
  return 0;
}
