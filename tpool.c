#include "tpool.h"


int ThreadPoolInit(struct ThreadPoolManager* t, int n){
    if (n < 0){
        n = 0;
    }
    t->totalThreads = n;

}

void ThreadPoolDestroy(struct ThreadPoolManager* t){}

int ThreadPoolInsertTask(struct ThreadPoolManager* t, struct Task* task){}

static void* threadPoolCheck(void* threadPool);

static int taskQueue_init(struct qTask* q){
    q->lenght = 0;
    q->start = NULL;
    q->end = NULL;

    pthread_mutex_init(&(q->qlock), NULL);

    return 0;
}
static void taskQueue_clear(struct qTask* q){
    while(q->lenght){
        free(taskQueue_pull(q));
    }

    q->start = NULL;
    q->end = NULL;
}

static void taskQueue_push(struct qTask* q, struct task* task){
    pthread_mutex_lock()
}

static struct task* taskQueue_pull(struct qTask* q){}

static void taskQueue_deleteAll(struct qTask* q){
    taskQueue_clear(q);
    pthread_mutex_destroy(&q->qlock);
}

