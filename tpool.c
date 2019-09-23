#include "tpool.h"


int ThreadPoolInit(ThreadPoolManager* t, int n){
    if (n < 0){
        n = 0;
    }
    t->totalThreads = n;

    if(taskQueue_init(&t->taskQueue) == -1){
        perror(" Task Queue Init Faild.\n");
        free(t);
        return -1;
    }

    if ((t->threads = (pthread_t*)malloc(n * sizeof(pthread_t)))==NULL){
        perror("Faild allocating memory.\n");
        taskQueue_deleteAll(&t->taskQueue);
        free(t);
        return -1;
    }

    pthread_mutex_init(&(t->plock), NULL);
    pthread_cond_init(&t->pcond, NULL);

    for( int i=0; i < n; i++){
        if(pthread_create(&t->threads[i], NULL, threadPoolCheck, t))
            return -1;
    }

    printf("Successful initialized ThreadPool.\n");
    return 0;
}

void ThreadPoolDestroy(ThreadPoolManager* t){}

int ThreadPoolInsertTask(ThreadPoolManager* t, task* task){}

static void* threadPoolCheck(void* threadPool){
    ThreadPoolManager* tmp_mr = (ThreadPoolManager*)threadPool;
    while(1){
        pthread_mutex_lock(&tmp_mr->plock);
        while(tmp_mr->taskQueue.lenght ==0){
            pthread_cond_wait(&tmp_mr->pcond, &tmp_mr->plock);
        }

        tmp_mr->numOfCurrThreads++;
        pthread_mutex_unlock(&tmp_mr->plock);

        void*(*new_f)(void* arg);
        void* new_arg;
        struct task* new_task;

        new_task= taskQueue_pop(&tmp_mr->taskQueue);
        if(new_task){
            new_f = new_task->f;
            new_arg = new_task->arg;
            new_f(new_arg);
                free(new_task->arg);
                free(new_task);
        }

        pthread_mutex_lock(&tmp_mr->plock);
        tmp_mr->numOfCurrThreads--;
        pthread_mutex_unlock(&tmp_mr->plock);
    }

    return NULL;
}

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
    pthread_mutex_lock(&q->qlock);
    task->next = NULL;

    if(q->lenght){       /*queue not empty*/
        q->end->next = task;
        q->end = task;
    }
    else{               /*queue is empty*/
        q->start = task;
        q->end = task;

    }

    q->lenght++;
    pthread_mutex_unlock(&q->qlock);
}

static struct task* taskQueue_pop( qTask* q){
    pthread_mutex_lock(&q->qlock);
    task* tmp_task = q->start;

    if (q->lenght == 1){
        q->end = NULL;
        q->start = NULL;
        q->lenght = 0;
    }

    if (q->lenght > 1){
        q->start = tmp_task->next;
        q->lenght--;
    }

 pthread_mutex_unlock(&q->qlock);
 return tmp_task;
}

static void taskQueue_deleteAll( qTask* q){
    taskQueue_clear(q);
    pthread_mutex_destroy(&q->qlock);
}

