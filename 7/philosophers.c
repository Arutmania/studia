#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static struct philosopher {
    char const* const name;
    pthread_mutex_t   mutex;
    enum state        state;
} PHILOSOPHERS[] = {
    { "Plato"     },
    { "Confucius" },
    { "Socrates"  },
    { "Voltaire"  },
    { "Descartes" },
};
#pragma GCC diagnostic pop

static pthread_mutex_t mutex;

static inline void test(int i) {
    int const l = (i + LEN(PHILOSOPHERS) - 1) % LEN(PHILOSOPHERS);
    int const r = (i + 1) % LEN(PHILOSOPHERS);

    if (PHILOSOPHERS[i].state == HUNGRY &&
        PHILOSOPHERS[l].state != EATING &&
        PHILOSOPHERS[r].state != EATING) {
        PHILOSOPHERS[i].state = EATING;
        pthread_mutex_unlock(&PHILOSOPHERS[i].mutex);
    }
}
static inline void grab_forks(int i) {
    pthread_mutex_lock(&mutex);
    PHILOSOPHERS[i].state = HUNGRY;
    test(i);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_lock(&PHILOSOPHERS[i].mutex);
}

static inline void put_away_forks(int i) {
    int const l = (i + LEN(PHILOSOPHERS) - 1) % LEN(PHILOSOPHERS);
    int const r = (i + 1) % LEN(PHILOSOPHERS);

    pthread_mutex_lock(&mutex);
    PHILOSOPHERS[i].state = THINKING;
    test(l);
    test(r);
    pthread_mutex_unlock(&mutex);
}

static inline void think(int i) {
    printf("%s is thinking\n", PHILOSOPHERS[i].name);
    sleep(1);
}

static inline void eat(int i, int meal) {
    printf("%s is eating for %d time\n", PHILOSOPHERS[i].name, meal);
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
    for (int i = 0; i < LEN(PHILOSOPHERS); ++i) {
        if (pthread_mutex_init(&PHILOSOPHERS[i].mutex, NULL) != 0)
            err(EXIT_FAILURE,
                "failed to initialize philosopher '%s' mutex",
                PHILOSOPHERS[i].name);
        /* initialize locked */
        pthread_mutex_lock(&PHILOSOPHERS[i].mutex);
    }

    /* create threads */
    pthread_t threads[LEN(PHILOSOPHERS)];
    int ids[LEN(PHILOSOPHERS)];
    for (int i = 0; i < LEN(PHILOSOPHERS); ++i) {
        ids[i] = i;
        if (pthread_create(&threads[i], NULL, philosopher, &ids[i]) != 0)
            err(EXIT_FAILURE,
                "failed to create thread for philosopher '%s'",
                PHILOSOPHERS[i].name);
    }

    /*
     * not much you can do when join, or destroy fails
     * second argument is address to retval
     */
    for (int i = 0; i < LEN(PHILOSOPHERS); ++i)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < LEN(PHILOSOPHERS); ++i)
        pthread_mutex_destroy(&PHILOSOPHERS[i].mutex);
#pragma GCC diagnostic pop
}
