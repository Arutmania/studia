#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>

static char const* const PHILOSOPHERS[] = {
    "Plato",
    "Confucius",
    "Socrates",
    "Voltaire",
    "Descartes",
};

#define LEN(x) (sizeof(x) / sizeof(x[0]))
#ifndef MEALS
#   define MEALS 5
#endif

static int SEMID;
static pid_t children[LEN(PHILOSOPHERS)];
static int created = 0;

/* terminate created processes and exit upon process creation failure */
static inline void handle_fail(void) {
    fputs("failed to create a child - exiting", stderr);
    for (; created > 0; --created)
        kill(children[created], SIGTERM);
    exit(EXIT_FAILURE);
}


static inline void grab_forks(int const left) {
    int const right = left + 1 % LEN(PHILOSOPHERS);

    printf("%s is trying to grab forks: %d and %d\n",
           PHILOSOPHERS[left], left, right);

    /*
     * decrease left and right fork semaphores
     *
     * because either both semaphores are changed
     * or none there is risk of deadlocks
     */
    semop(
        SEMID,
        (struct sembuf[]) {
            { .sem_num = left,  .sem_op = -1, .sem_flg = 0 },
            { .sem_num = right, .sem_op = -1, .sem_flg = 0 },
        },
        2
    );

    printf("%s grabbed forks: %d, %d\n",
           PHILOSOPHERS[left], left, right);
}

static inline void put_away_forks(int const left) {
    int const right = left + 1 % LEN(PHILOSOPHERS);

    printf("%s is putting away forks: %d and %d\n",
           PHILOSOPHERS[left], left, right);

    /*
     * put away forks by increasing left and right fork semaphores
     */
    semop(
        SEMID,
        (struct sembuf[2]) {
            { .sem_num = left,  .sem_op = 1, .sem_flg = 0 },
            { .sem_num = right, .sem_op = 1, .sem_flg = 0 },
        },
        2
    );
}

void think(int philosopher) {
    printf("%s is thinking\n", PHILOSOPHERS[philosopher]);
    sleep(1);
}

void eat(int philosopher, int meal) {
    printf("%s is eating for %d time\n", PHILOSOPHERS[philosopher], meal);
    sleep(1);
}

void philosopher(int id) {
    for (int i = 0; i < MEALS; ++i) {
        think(id);
        grab_forks(id);
        eat(id, i + 1);
        put_away_forks(id);
    }
    exit(EXIT_SUCCESS);
}

int main(void) {
    /* create LEN(PHILOSOPHERS) semaphores with rights 0666 */
    SEMID = semget(IPC_PRIVATE, LEN(PHILOSOPHERS), 0666);

    /* check if created successfully */
    if (SEMID < 0)
        err(EXIT_FAILURE, "failed to create semaphores");


    /* initialize all semaphores to 1 - all forks are free initially */
    for (int i = 0; i < (int)LEN(PHILOSOPHERS); ++i)
        if (semctl(SEMID, i, SETVAL, 1) < 0)
            err(EXIT_FAILURE, "failed to initialize semaphore %d", i);

    /* create LEN(PHILOSOPHERS) processes */
    while (created < (int)LEN(PHILOSOPHERS)) {
        pid_t child = fork();

        /*
         * if failed creating process terminate
         * all created previosly and exit
         *
         * if in the child process run the philosopher function
         * everything after the else if is run only in the parent process
         * because philosopher function exits
         */
        if (child < 0)
            handle_fail();
        else if (child == 0)
            philosopher(created);

        /*
         * in the parent process add child to the children list
         */
        children[created++] = child;
    }

    /* wait until all philosophers finish before unallocating semaphores */
    while(wait(NULL) > 0)
        continue;

    /* remove the allocated semaphores */
    if (semctl(SEMID, LEN(PHILOSOPHERS), IPC_RMID) < 0)
        err(EXIT_FAILURE, "failed to deallocate semaphores");
}
