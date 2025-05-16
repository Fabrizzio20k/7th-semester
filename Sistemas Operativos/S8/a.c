#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  int sillasDisponibles;
  int *turnos;
  sem_t cortandoCabello;
  pthread_cond_t barberoActivo;
  volatile int pos;
  volatile int id;

} mostaza;

void init(int sillas, mostaza *m) {
  m->sillasDisponibles = sillas;
  m->turnos = malloc(sillas * sizeof(int));
  sem_init(&m->cortandoCabello, 0, 1);
  pthread_cond_init(&m->barberoActivo, NULL);
  m->pos = 0;
  m->id = 0;
}

void haircut(mostaza *m) {
  sem_wait(&m->cortandoCabello);
  pthread_cond_signal(&m->barberoActivo);

  sem_post(&m->cortandoCabello);
}

void next_customer(mostaza *m) {}

int main() {}