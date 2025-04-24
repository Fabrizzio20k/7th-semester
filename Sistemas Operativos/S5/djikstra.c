#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_THREADS 15

int counter = 0;
int order = -1;
int interest[NUM_THREADS];

int checkOthersAreTrue(int me) {
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (i != me && interest[i] == 1)
        {
            return 1;
        }
    }
    return 0;
}

void* djikstra(void* arg) {
    int me = *(int*)arg;
    int o1 = 1;
    int o = 0;

    for (int i = 0; i < 100000; i++)
    {
        __atomic_store(&interest[me], &o1, __ATOMIC_SEQ_CST);
        while (checkOthersAreTrue(me))
        {
            if (order != me)
            {
                __atomic_store(&interest[me], &o, __ATOMIC_SEQ_CST);
                while (order != -1);
                order = me;
                __atomic_store(&interest[me], &o1, __ATOMIC_SEQ_CST);
            }
        }

        counter++;
        order = -1;
        __atomic_store(&interest[me], &o, __ATOMIC_SEQ_CST);
    }
    
}



int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        interest[i] = 0;
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, djikstra, (void*)&thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }


    printf("Final counter value: %d\n", counter);

    return 0;
}