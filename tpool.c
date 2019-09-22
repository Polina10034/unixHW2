#include "tpool.h"


int ThreadPoolInit(ThreadPoolManager* t, int n){
    if (n < 0){
        n = 0;
    }
    t->totalThreads = n;

}

void ThreadPoolDestroy(ThreadPoolManager* t){0}

int ThreadPoolInsertTask(ThreadPoolManager* t, task* task){}

static void* threadPoolCheck(void* threadPool){}

static int taskQueue_init(qTask* q){
    q->lenght = 0;
    q->start = NULL;
    q->end = NULL;

    pthread_mutex_init(&(q->qlock), NULL);

    return 0;
}
static void taskQueue_clear(qTask* q){
    while(q->lenght){
        free(taskQueue_pull(q));
    }

    q->start = NULL;
    q->end = NULL;
}

static void taskQueue_push( qTask* q,  task* task){
    pthread_mutex_lock()
}

static struct task* taskQueue_pull( qTask* q){}

static void taskQueue_deleteAll( qTask* q){
    taskQueue_clear(q);
    pthread_mutex_destroy(&q->qlock);
}

