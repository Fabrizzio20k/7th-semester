#include <stdio.h>
#include <stdlib.h>

volatile int s = 0, lock = 0;
void thread_0() {
  while (lock != 0)
    ;
  lock = 1;
  s = 0;
  printf("Thread 0: s = %d\n", s);
  lock = 0;
}

void thread_1() {
  while (lock != 0)
    ;
  lock = 1;
  s = 1;
  printf("Thread 1: s = %d\n", s);
  lock = 0;
}

int main() {
  thread_0();
  thread_1();
  return 0;
}