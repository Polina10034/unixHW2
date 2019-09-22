
#ifndef TPOOL_H
#define TPOOL_H

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/prctl.h>

using namespace std;

struct task{
    void*(*f)(void*);
    void* arg;
    struct task* next;
};

struct qTask{
    pthread_mutex_t qlock;
    task *start;
    task *end;
    int lenght;
};

struct ThreadPoolManager{
    pthread_t* threads;
    pthread_mutex_t plock;
    pthread_cond_t pcond;
    int numOfCurrThreads;
    int totalThreads;
    int* sockets;
    qTask Q;
};

int ThreadPoolInit(struct ThreadPoolManager* t, int n);
void ThreadPoolDestroy(struct ThreadPoolManager* t);
int ThreadPoolInsertTask(struct ThreadPoolManager* t, struct Task* task);

static int taskQueue_init(qTask*);
static void taskQueue_clear(qTask*);
static void taskQueue_push(qTask*, task*);
static struct task* taskQueue_pull(qTask*);
static void taskQueue_deleteAll(qTask*);
static void* threadPoolCheck(void* threadPool);



#endif