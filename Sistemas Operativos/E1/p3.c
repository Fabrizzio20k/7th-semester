#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_VEHICLES 3 // Máximo vehículos en el puente simultáneamente

atomic_int north_count = 0;      // Vehículos del norte esperando
atomic_int south_count = 0;      // Vehículos del sur esperando
atomic_int on_bridge = 0;        // Vehículos actualmente en el puente
atomic_int bridge_direction = 0; // 0=libre, 1=norte, 2=sur

sem_t bridge_mutex;            // Exclusión mutua para el puente
sem_t north_queue;             // Cola de vehículos del norte
sem_t south_queue;             // Cola de vehículos del sur
pthread_mutex_t monitor_mutex; // Monitor para control de acceso
pthread_cond_t north_cond;     // Condición para vehículos del norte
pthread_cond_t south_cond;     // Condición para vehículos del sur

void enter_bridge(int direction, int id) {
  pthread_mutex_lock(&monitor_mutex);

  if (direction == 1) { // Norte
    atomic_fetch_add(&north_count, 1);
    printf("Vehículo Norte %d llegó al puente (esperando: N=%d, S=%d)\n", id,
           (int)north_count, (int)south_count);

    // Esperar hasta poder cruzar
    while (bridge_direction == 2 ||
           (bridge_direction == 0 && south_count > 0 &&
            north_count > south_count) ||
           on_bridge >= MAX_VEHICLES) {
      pthread_cond_wait(&north_cond, &monitor_mutex);
    }

    atomic_fetch_sub(&north_count, 1);
    atomic_fetch_add(&on_bridge, 1);
    bridge_direction = 1;
    printf("Vehículo Norte %d ENTRA al puente (en puente: %d)\n", id,
           (int)on_bridge);

  } else { // Sur
    atomic_fetch_add(&south_count, 1);
    printf("Vehículo Sur %d llegó al puente (esperando: N=%d, S=%d)\n", id,
           (int)north_count, (int)south_count);

    // Esperar hasta poder cruzar
    while (bridge_direction == 1 ||
           (bridge_direction == 0 && north_count > 0 &&
            south_count > north_count) ||
           on_bridge >= MAX_VEHICLES) {
      pthread_cond_wait(&south_cond, &monitor_mutex);
    }

    atomic_fetch_sub(&south_count, 1);
    atomic_fetch_add(&on_bridge, 1);
    bridge_direction = 2;
    printf("Vehículo Sur %d ENTRA al puente (en puente: %d)\n", id,
           (int)on_bridge);
  }

  pthread_mutex_unlock(&monitor_mutex);
}

void exit_bridge(int direction, int id) {
  pthread_mutex_lock(&monitor_mutex);

  atomic_fetch_sub(&on_bridge, 1);
  printf("Vehículo %s %d SALE del puente (en puente: %d)\n",
         direction == 1 ? "Norte" : "Sur", id, (int)on_bridge);

  // Si no hay más vehículos en el puente, liberar dirección
  if (on_bridge == 0) {
    bridge_direction = 0;
    printf("Puente libre - cambiando dirección\n");
    pthread_cond_broadcast(&north_cond);
    pthread_cond_broadcast(&south_cond);
  } else {
    // Permitir más vehículos de la misma dirección
    if (direction == 1) {
      pthread_cond_signal(&north_cond);
    } else {
      pthread_cond_signal(&south_cond);
    }
  }

  pthread_mutex_unlock(&monitor_mutex);
}

void *north_vehicle(void *arg) {
  int id = *((int *)arg);

  while (1) {
    sleep(2 + rand() % 4); // Tiempo antes de llegar al puente

    enter_bridge(1, id);

    printf("Vehículo Norte %d cruzando...\n", id);
    sleep(2 + rand() % 3); // Tiempo cruzando el puente

    exit_bridge(1, id);

    sleep(3 + rand() % 5); // Tiempo antes del siguiente viaje
  }
}

void *south_vehicle(void *arg) {
  int id = *((int *)arg);

  while (1) {
    sleep(2 + rand() % 4); // Tiempo antes de llegar al puente

    enter_bridge(2, id);

    printf("Vehículo Sur %d cruzando...\n", id);
    sleep(2 + rand() % 3); // Tiempo cruzando el puente

    exit_bridge(2, id);

    sleep(3 + rand() % 5); // Tiempo antes del siguiente viaje
  }
}

int main() {
  const int N_VEHICLES = 4;
  pthread_t north_threads[N_VEHICLES];
  pthread_t south_threads[N_VEHICLES];
  int north_ids[N_VEHICLES];
  int south_ids[N_VEHICLES];

  // Inicializar semáforos y monitor
  sem_init(&bridge_mutex, 0, 1);
  sem_init(&north_queue, 0, 0);
  sem_init(&south_queue, 0, 0);
  pthread_mutex_init(&monitor_mutex, NULL);
  pthread_cond_init(&north_cond, NULL);
  pthread_cond_init(&south_cond, NULL);

  // Crear vehículos del norte
  for (int i = 0; i < N_VEHICLES; i++) {
    north_ids[i] = i + 1;
    pthread_create(&north_threads[i], NULL, north_vehicle, &north_ids[i]);
  }

  // Crear vehículos del sur
  for (int i = 0; i < N_VEHICLES; i++) {
    south_ids[i] = i + 1;
    pthread_create(&south_threads[i], NULL, south_vehicle, &south_ids[i]);
  }

  // Esperar threads (bucle infinito en la práctica)
  for (int i = 0; i < N_VEHICLES; i++) {
    pthread_join(north_threads[i], NULL);
    pthread_join(south_threads[i], NULL);
  }

  // Limpiar recursos
  sem_destroy(&bridge_mutex);
  sem_destroy(&north_queue);
  sem_destroy(&south_queue);
  pthread_mutex_destroy(&monitor_mutex);
  pthread_cond_destroy(&north_cond);
  pthread_cond_destroy(&south_cond);

  return 0;
}