#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <pthread.h>
#include <unistd.h>

#define LEN(x) (sizeof(x) / sizeof(x[0]))

#ifndef MEALS
#   define MEALS 5
#endif

/*
 * Tannenbaum's Solution
 */

enum state { THINKING, HUNGRY, EATING };

/*
 * mutexes are initialized in main, missing state initializer makes so it is
 * zero initialized - to thinking state
 */
static struct philosopher {
    char const* const name;
    pthread_mutex_t   mutex;
    enum state        state;
} philosophers[] = {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    { "Plato"     },
    { "Confucius" },
    { "Socrates"  },
    { "Voltaire"  },
    { "Descartes" },
#pragma GCC diagnostic pop
};

/* mutex for critical section */
static pthread_mutex_t mutex;

static inline void test(int i) {
    /* get index to the left and right of i */
    int const l = (i + LEN(philosophers) - 1) % LEN(philosophers);
    int const r = (i + 1) % LEN(philosophers);

    /*
     * if possible (the states are right) unlock philosopher mutex
     * unlocked mutex means he is using forks
     */
    if (philosophers[i].state == HUNGRY &&
        philosophers[l].state != EATING &&
        philosophers[r].state != EATING) {
        philosophers[i].state = EATING;
        pthread_mutex_unlock(&philosophers[i].mutex);
    }
}
static inline void grab_forks(int i) {
    /* lock mutex before critical section */
    pthread_mutex_lock(&mutex);
    /* indicate intent */
    philosophers[i].state = HUNGRY;
    /* test conditionally unlocks philosopher given by i - means he is eating */
    test(i);
    /* if test didn't unlock the philosopher mutex go to sleep */
    pthread_mutex_unlock(&mutex);
    /* unlock mutex after critical section */
    pthread_mutex_lock(&philosophers[i].mutex);
}

static inline void put_away_forks(int i) {
    /* indexes of philosophers to the left and right */
    int const l = (i + LEN(philosophers) - 1) % LEN(philosophers);
    int const r = (i + 1) % LEN(philosophers);

    /* critical section */
    pthread_mutex_lock(&mutex);
    /* intent */
    philosophers[i].state = THINKING;
    test(l);
    test(r);
    /* end of critical section */
    pthread_mutex_unlock(&mutex);
}

static inline void think(int i) {
    printf("%s is thinking\n", philosophers[i].name);
    sleep(1);
}

static inline void eat(int i, int meal) {
    printf("%s is eating for %d time\n", philosophers[i].name, meal);
    sleep(1);
}

static inline void* philosopher(void* arg) {
    int const id = *(int*)arg;
    for (int i = 0; i < MEALS; ++i) {
        think(id);
        grab_forks(id);
        eat(id, i + 1);
        put_away_forks(id);
    }
    return NULL;
}

int main(void) {
    /* initialize mutexes */
    if (pthread_mutex_init(&mutex, NULL) != 0)
        err(EXIT_FAILURE, "failed to initialize global mutex");

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
    /* initialize mutexes - locked means not eating */
    for (int i = 0; i < LEN(philosophers); ++i) {
        if (pthread_mutex_init(&philosophers[i].mutex, NULL) != 0)
            err(EXIT_FAILURE,
                "failed to initialize philosopher '%s' mutex",
                philosophers[i].name);
        /* initialize locked */
        pthread_mutex_lock(&philosophers[i].mutex);
    }

    /* create threads */
    pthread_t threads[LEN(philosophers)];
    /* ids passed to threads must be stored somewhere */
    int ids[LEN(philosophers)];
    for (int i = 0; i < LEN(philosophers); ++i) {
        ids[i] = i;
        if (pthread_create(&threads[i], NULL, philosopher, &ids[i]) != 0)
            err(EXIT_FAILURE,
                "failed to create thread for philosopher '%s'",
                philosophers[i].name);
    }

    /*
     * not much you can do when join, or destroy fails
     * second argument is address to thread function return value
     */
    for (int i = 0; i < LEN(philosophers); ++i)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < LEN(philosophers); ++i)
        pthread_mutex_destroy(&philosophers[i].mutex);
#pragma GCC diagnostic pop
}
