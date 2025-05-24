/*
ANÁLISIS DEL PROBLEMA:
- 3 Pasteleros: A(harina+huevos), B(huevos+azúcar), C(azúcar+harina)
- 1 Chef: coloca 2 ingredientes distintos aleatoriamente
- Recursos: harina, huevos, azúcar
- Restricciones: chef no repone hasta que pastelero termine

DIAGRAMA DE TIEMPO SIMPLE:
t0: Chef coloca harina+huevos -> Pastelero A puede trabajar
t1: A toma ingredientes y cocina
t2: A termina -> Chef puede reponer
t3: Chef coloca huevos+azúcar -> Pastelero B puede trabajar
...

ANÁLISIS DE DEADLOCK:
- Recursos: 3 ingredientes (harina, huevos, azúcar)
- Procesos: 3 pasteleros + 1 chef
- No hay deadlock porque:
  * Solo un pastelero puede trabajar a la vez (por diseño del problema)
  * Chef espera a que termine el pastelero activo
  * No hay dependencias circulares
*/

#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum { HARINA = 0, HUEVOS = 1, AZUCAR = 2 } ingredient_t;
const char *ingredient_names[] = {"Harina", "Huevos", "Azúcar"};

atomic_int ingredients_on_table[3] = {0, 0, 0}; // Ingredientes disponibles
atomic_int chef_can_place = 1; // Chef puede colocar ingredientes

pthread_mutex_t table_monitor; // Monitor para la mesa
pthread_cond_t chef_cond;      // Chef puede colocar ingredientes
pthread_cond_t baker_cond[3];  // Cada pastelero puede trabajar

int can_baker_work(int baker_id) {
  // Pastelero A (0): necesita harina+huevos (indices 0,1)
  // Pastelero B (1): necesita huevos+azúcar (indices 1,2)
  // Pastelero C (2): necesita azúcar+harina (indices 2,0)

  if (baker_id == 0) {
    return ingredients_on_table[HARINA] && ingredients_on_table[HUEVOS];
  } else if (baker_id == 1) {
    return ingredients_on_table[HUEVOS] && ingredients_on_table[AZUCAR];
  } else {
    return ingredients_on_table[AZUCAR] && ingredients_on_table[HARINA];
  }
}

void clear_table() {
  atomic_store(&ingredients_on_table[HARINA], 0);
  atomic_store(&ingredients_on_table[HUEVOS], 0);
  atomic_store(&ingredients_on_table[AZUCAR], 0);
}

void *chef(void *arg) {
  while (1) {
    pthread_mutex_lock(&table_monitor);

    // Esperar hasta poder colocar ingredientes
    while (!chef_can_place) {
      printf("Chef esperando...\n");
      pthread_cond_wait(&chef_cond, &table_monitor);
    }

    // Elegir 2 ingredientes distintos aleatoriamente
    ingredient_t first = rand() % 3;
    ingredient_t second;
    do {
      second = rand() % 3;
    } while (second == first);

    // Colocar ingredientes en la mesa
    atomic_store(&ingredients_on_table[first], 1);
    atomic_store(&ingredients_on_table[second], 1);
    atomic_store(&chef_can_place, 0);

    printf("Chef coloca: %s + %s\n", ingredient_names[first],
           ingredient_names[second]);

    // Despertar a todos los pasteleros para que verifiquen
    for (int i = 0; i < 3; i++) {
      pthread_cond_signal(&baker_cond[i]);
    }

    pthread_mutex_unlock(&table_monitor);
    sleep(1);
  }
}

void *baker(void *arg) {
  int id = *((int *)arg);
  const char *baker_names[] = {"A(harina+huevos)", "B(huevos+azúcar)",
                               "C(azúcar+harina)"};

  while (1) {
    pthread_mutex_lock(&table_monitor);

    // Esperar hasta que pueda trabajar
    while (!can_baker_work(id)) {
      printf("Pastelero %s esperando sus ingredientes...\n", baker_names[id]);
      pthread_cond_wait(&baker_cond[id], &table_monitor);
    }

    // Tomar ingredientes
    printf("Pastelero %s toma los ingredientes y comienza a cocinar\n",
           baker_names[id]);
    clear_table();

    pthread_mutex_unlock(&table_monitor);

    // Cocinar (fuera del monitor)
    printf("Pastelero %s cocinando...\n", baker_names[id]);
    sleep(3 + rand() % 3);

    // Terminar y notificar al chef
    pthread_mutex_lock(&table_monitor);
    printf("Pastelero %s terminó su postre\n", baker_names[id]);
    atomic_store(&chef_can_place, 1);
    pthread_cond_signal(&chef_cond);
    pthread_mutex_unlock(&table_monitor);

    sleep(2 + rand() % 4); // Descanso antes del siguiente postre
  }
}

int main() {
  pthread_t chef_thread;
  pthread_t baker_threads[3];
  int baker_ids[3] = {0, 1, 2};

  // Inicializar monitor
  pthread_mutex_init(&table_monitor, NULL);
  pthread_cond_init(&chef_cond, NULL);
  for (int i = 0; i < 3; i++) {
    pthread_cond_init(&baker_cond[i], NULL);
  }

  // Crear threads
  pthread_create(&chef_thread, NULL, chef, NULL);
  for (int i = 0; i < 3; i++) {
    pthread_create(&baker_threads[i], NULL, baker, &baker_ids[i]);
  }

  // Esperar threads
  pthread_join(chef_thread, NULL);
  for (int i = 0; i < 3; i++) {
    pthread_join(baker_threads[i], NULL);
  }

  // Limpiar recursos
  pthread_mutex_destroy(&table_monitor);
  pthread_cond_destroy(&chef_cond);
  for (int i = 0; i < 3; i++) {
    pthread_cond_destroy(&baker_cond[i]);
  }

  return 0;
}