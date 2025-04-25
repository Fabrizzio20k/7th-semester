#include <stdio.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/futex.h>
#include <stdatomic.h>

#define N_THREADS 5
#define N_TRANSFERS 10000000
#define N_ACCOUNTS 10

void futex_wait (int *addr, int val) {
    syscall(SYS_futex, addr, FUTEX_WAIT, val, NULL, NULL, 0);
}

void futex_wake (int *addr) {
    syscall(SYS_futex, addr, FUTEX_WAKE, 1, NULL, NULL, 0);
}


int lock = 0;
unsigned int accounts[N_ACCOUNTS] = { 0 };

void *transfer(void *arg) {
    while (atomic_load(&lock) != 0) {
        futex_wait(&lock, 0);
    }

    atomic_store(&lock, 1);

    int amount = *((int *) arg);

    for (int i = 0; i < N_TRANSFERS; ++i) {
        for (int j = 0; j < N_ACCOUNTS; ++j) {
            accounts[j] += amount;
        }
    }

    atomic_store(&lock, 0);
    futex_wake(&lock);

    return NULL;
}

void print_accounts () {
	unsigned int total = 0;

	for (int i = 1; i <= N_THREADS; ++i) {
		total += i * 10 * N_TRANSFERS;
	}

	printf ("Each account should have a balance of: %8d\n", total);

	for (int i = 0; i < N_ACCOUNTS; ++i) {
		printf ("Account %2d: %5d\n", i, accounts[i]);
	}
}

int main () {
    pthread_t threads[N_THREADS];
    int amounts[N_THREADS];

    for (int i = 0; i < N_THREADS; ++i) {
        amounts[i] = (i + 1) * 10;
        pthread_create (&threads[i], NULL, transfer, (void *) &amounts[i]);
    }

    for (int i = 0; i < N_THREADS; ++i) {
        pthread_join (threads[i], NULL);
    }
	
    print_accounts ();
}
