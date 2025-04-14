#include <pthread.h>
#include <stdio.h>
long int counter = 0;
#define BIG 10000000

void *increase_counter(void *arg) {
  for (size_t i = 0; i < BIG; i++) {
    counter++;
  }
}

int main() {
  pthread_t t1, t2;
  pthread_create(&t1, NULL, increase_counter, NULL);
  pthread_create(&t2, NULL, increase_counter, NULL);

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);

  printf("Counter: %ld\n", counter);
}