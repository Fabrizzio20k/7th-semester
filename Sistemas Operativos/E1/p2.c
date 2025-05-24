#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int N = 5;             // Número de filósofos
sem_t forks[N];              // Semáforos para tenedores
sem_t mutex;                 // Exclusión mutua
int unlucky_philosopher = 0; // Filósofo que sufrirá starvation

void *philosopher(void *arg) {
  int id = *((int *)arg);
  int left = id;
  int right = (id + 1) % N;

  while (1) {
    printf("Filósofo %d pensando...\n", id);
    sleep(1 + rand() % 2); // Otros filósofos piensan menos tiempo

    printf("Filósofo %d tiene hambre\n", id);

    if (id == unlucky_philosopher) {
      // Filósofo unlucky usa estrategia que causa starvation
      printf("Filósofo %d esperando tenedor derecho %d...\n", id, right);
      sem_wait(&forks[right]);
      printf("Filósofo %d tomó tenedor derecho %d\n", id, right);
      sleep(4); // Espera mucho tiempo, permitiendo que otros monopolicen

      printf("Filósofo %d esperando tenedor izquierdo %d...\n", id, left);
      sem_wait(&forks[left]);
      printf("Filósofo %d FINALMENTE tomó tenedor izquierdo %d\n", id, left);
    } else {
      // Otros filósofos son más agresivos y eficientes
      sem_wait(&forks[left]);
      printf("Filósofo %d tomó tenedor izquierdo %d\n", id, left);
      sem_wait(&forks[right]);
      printf("Filósofo %d tomó tenedor derecho %d\n", id, right);
    }

    printf("Filósofo %d comiendo...\n", id);
    sleep(2 + rand() % 2);

    sem_post(&forks[left]);
    sem_post(&forks[right]);
    printf("Filósofo %d terminó de comer\n", id);

    sleep(1);
  }
}

int main() {
  pthread_t philosophers[N];
  int philosopher_ids[N];

  for (int i = 0; i < N; i++) {
    sem_init(&forks[i], 0, 1); // Cada tenedor disponible
  }
  sem_init(&mutex, 0, 1);

  for (int i = 0; i < N; i++) {
    philosopher_ids[i] = i;
    pthread_create(&philosophers[i], NULL, philosopher, &philosopher_ids[i]);
  }

  for (int i = 0; i < N; i++) {
    pthread_join(philosophers[i], NULL);
  }

  for (int i = 0; i < N; i++) {
    sem_destroy(&forks[i]);
  }
  sem_destroy(&mutex);

  return 0;
}