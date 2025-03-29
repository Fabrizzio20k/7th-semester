#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (fork() == 0) {
    return 0;
  } else {
    sleep(10);
  }
}