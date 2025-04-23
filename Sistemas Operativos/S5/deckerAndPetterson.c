#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

int counter = 0;
int order = 0;

int interest[2];

void* increment1(void* arg) {
    if (strcmp((char*)arg, "d") == 0){
        printf("Decker en t1\n");
        for (int i = 0; i < 100000; i++)
        {
            __atomic_fetch_add (&interest[0], 1, __ATOMIC_SEQ_CST);
            while (interest[1])
            {
                if(order != 0)
                {
                    __atomic_fetch_sub (&interest[0], 1, __ATOMIC_SEQ_CST);
                    while (order != 0);
                    __atomic_fetch_add (&interest[0], 1, __ATOMIC_SEQ_CST);
                }
    
            }
    
            counter++;
    
            order = 1;
            __atomic_fetch_sub (&interest[0], 1, __ATOMIC_SEQ_CST);
        }
    }
    else if (strcmp((char*)arg, "p") == 0){
        printf("Petterson en t1\n");
        int o = 1;
        for (int i = 0; i < 100000; i++){
            __atomic_fetch_add (&interest[0], 1, __ATOMIC_SEQ_CST);
            __atomic_store(&order, &o, __ATOMIC_SEQ_CST);
            while(interest[1] && order != 0);
            counter++;
             __atomic_fetch_sub (&interest[0], 1, __ATOMIC_SEQ_CST);
            
        }
    }
}
void* increment2(void* arg) {
    if (strcmp((char*)arg, "d") == 0){
        printf("Decker en t2\n");
        for (int i = 0; i < 100000; i++)
        {
            __atomic_fetch_add (&interest[1], 1, __ATOMIC_SEQ_CST);
            while (interest[0])
            {
                if(order != 1)
                {
                    __atomic_fetch_sub (&interest[1], 1, __ATOMIC_SEQ_CST);
                    while (order != 1);
                    __atomic_fetch_add (&interest[1], 1, __ATOMIC_SEQ_CST);
                }

            }
            counter++;

            order = 0;
            __atomic_fetch_sub (&interest[1], 1, __ATOMIC_SEQ_CST);
        }
    }
    else if (strcmp((char*)arg, "p") == 0){
        int o = 0;
        printf("Petterson en t2\n");
        for (int i = 0; i < 100000; i++){
            __atomic_fetch_add (&interest[1], 1, __ATOMIC_SEQ_CST);
            __atomic_store(&order, &o, __ATOMIC_SEQ_CST);
            while(interest[0] && order != 1);
            counter++;
             __atomic_fetch_sub (&interest[1], 1, __ATOMIC_SEQ_CST);
        }
    }
}

int main() {
    interest[0] = 0;
    interest[1] = 0;
    char* arg1 = "p";
    pthread_t thread1, thread2;

    pthread_create(&thread1, NULL, increment1, (void*)arg1);
    pthread_create(&thread2, NULL, increment2, (void*)arg1);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("Final counter value: %d\n", counter);

    return 0;
}