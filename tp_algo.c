#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

#define NUM_CHILDREN 4

sem_t *sem;

void handle_sigusr1(int sig) {
    printf("Process %d received SIGUSR1\n", getpid());
}

void handle_sigusr2(int sig) {
    printf("Process %d received SIGUSR2\n", getpid());
}

void child_process(int child_num) {
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);
    sem_wait(sem);
    printf("Child %d starting task\n", child_num);
    sleep(1 + child_num);
    printf("Child %d completed task\n", child_num);
    kill(getppid(), SIGUSR1);
    exit(0);
}

void handle_confirmation(int sig) {
    static int confirmations = 0;
    confirmations++;
    printf("Parent received confirmation %d\n", confirmations);
    if (confirmations == NUM_CHILDREN) {
        sem_unlink("/semaphore_example");
        sem_close(sem);
    }
}

int main() {
    pid_t pids[NUM_CHILDREN];
    int i;
    sem = sem_open("/semaphore_example", O_CREAT, 0644, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, handle_confirmation);
    for (i = 0; i < NUM_CHILDREN; i++) {
        if ((pids[i] = fork()) < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            child_process(i + 1);
        }
    }

    sleep(1); 
    for (i = 0; i < NUM_CHILDREN; i++) {
        sem_post(sem);
    }
    for (i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }
    sem_close(sem);
    sem_unlink("/semaphore_example");
    return 0;
}
