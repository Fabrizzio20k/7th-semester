#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

volatile int H = 5;     // Capacidad del tarro
atomic_int counter = 0; // Contador atómico de porciones
const int n = 10;       // Número de abejas
sem_t mutex;            // Exclusión mutua
sem_t bear_sem;         // Despertar al oso
sem_t abeja_sem;        // Control de abejas trabajando

void *abeja(void *arg) {
  int id = *((int *)arg);
  while (1) {
    sem_wait(&abeja_sem);
    sem_wait(&mutex);

    if (counter < H) {
      int current_count =
          atomic_fetch_add(&counter, 1) + 1; // Incremento atómico
      printf("Abeja %d: contador = %d\n", id, current_count);

      if (current_count == H) {
        printf("¡Tarro lleno! Abeja %d despierta al oso\n", id);
        sem_post(&bear_sem); // Despertar al oso
      } else {
        sem_post(&abeja_sem); // Permitir siguiente abeja
      }
    } else {
      sem_post(&abeja_sem); // Devolver permiso si tarro lleno
    }

    sem_post(&mutex);
    sleep(1 + rand() % 2);
  }
}

void *oso(void *arg) {
  while (1) {
    printf("Oso durmiendo...\n");
    sem_wait(&bear_sem);

    sem_wait(&mutex);
    printf("Oso despierta y come toda la miel (%d porciones)\n", (int)counter);
    sleep(2);
    atomic_store(&counter, 0); // Vaciar tarro atómicamente
    printf("Oso terminó de comer. Tarro vacío.\n");

    for (int i = 0; i < H; i++) { // Liberar permisos para H abejas
      sem_post(&abeja_sem);
    }

    sem_post(&mutex);
    sleep(1);
  }
}

int main(int argc, char *argv[]) {
  pthread_t abeja_thread[n];
  pthread_t oso_thread;
  int abeja_id[n];

  sem_init(&mutex, 0, 1);     // Mutex en 1
  sem_init(&bear_sem, 0, 0);  // Oso duerme inicialmente
  sem_init(&abeja_sem, 0, H); // H abejas pueden trabajar

  pthread_create(&oso_thread, NULL, oso, NULL);

  for (int i = 0; i < n; i++) {
    abeja_id[i] = i + 1;
    if (pthread_create(&abeja_thread[i], NULL, abeja, &abeja_id[i]) != 0) {
      perror("Failed to create thread");
      exit(EXIT_FAILURE);
    }
  }

  pthread_join(oso_thread, NULL);
  for (int i = 0; i < n; i++) {
    pthread_join(abeja_thread[i], NULL);
  }

  sem_destroy(&mutex);
  sem_destroy(&bear_sem);
  sem_destroy(&abeja_sem);

  return 0;
}