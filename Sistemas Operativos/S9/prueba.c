#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  int direccion;
  int peso_max;
  int *orden;
  int peso_actual;
  int max_cantidad_autos;
  int d1, d2;

  int pos;
  int head;

  pthread_mutex_t lock;
  pthread_cond_t lleno, direccion_opuesta, turno;
  sem_t counter_autos;

} puente;

puente *p;

void init(puente *p, int peso, int max_cantidad_autos) {
  p->direccion = 0;
  p->peso_max = 0;
  p->peso_actual = 0;
  p->max_cantidad_autos = max_cantidad_autos;
  p->pos = 0;
  p->head = 0;
  p->d1 = 0;
  p->d2 = 0;

  p->orden = malloc(peso * sizeof(int));
  for (int i = 0; i < max_cantidad_autos; i++) {
    p->orden[i] = -1;
  }

  pthread_mutex_init(&p->lock, NULL);
  pthread_cond_init(&p->lleno, NULL);
  pthread_cond_init(&p->turno, NULL);
  pthread_cond_init(&p->direccion_opuesta, NULL);
  sem_init(&p->counter_autos, 0, max_cantidad_autos);
}

void solicitar_entrada_puente(int direccion_vehiculo, int peso_vehiculo,
                              int id_vehiculo) {
  sem_wait(&p->counter_autos);

  pthread_mutex_lock(&p->lock);
  while (p->orden[p->pos] != -1) {
    p->pos = (p->pos + 1) % p->max_cantidad_autos;
  }
  p->orden[p->pos] = id_vehiculo;

  while (p->orden[p->head] != id_vehiculo) {
    pthread_cond_wait(&p->turno, &p->lock);
  }

  if (direccion_vehiculo == 0) {
    while (p->d2 > 0) {
      pthread_cond_wait(&p->direccion_opuesta, &p->lock);
    }

  } else {
    while (p->d1 > 0) {
      pthread_cond_wait(&p->direccion_opuesta, &p->lock);
    }
  }

  while (peso_vehiculo + p->peso_actual > p->peso_max) {
    pthread_cond_wait(&p->lleno, &p->lock);
  }

  p->direccion = direccion_vehiculo;
  p->peso_actual += peso_vehiculo;

  if (direccion_vehiculo == 0) {
    p->d1++;
  } else {
    p->d2++;
  }

  pthread_mutex_unlock(&p->lock);
}

void salir_del_puente(int direccion_vehiculo, int peso_vehiculo,
                      int id_vehiculo) {
  pthread_mutex_lock(&p->lock);
  p->head = (p->head + 1) % p->max_cantidad_autos;

  if (direccion_vehiculo == 0) {
    p->d1--;
  } else {
    p->d2--;
  }

  p->peso_actual -= peso_vehiculo;
  pthread_cond_broadcast(&p->lleno);
  pthread_cond_broadcast(&p->direccion_opuesta);
  pthread_cond_broadcast(&p->turno);
  sem_post(&p->counter_autos);
  pthread_mutex_unlock(&p->lock);
}

int main() {}