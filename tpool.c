#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <sys/prctl.h>
#include "tpool.h"

static void* threadPoolCheck(void* threadPool);

static int   taskQueue_init(qTask* q);
static void  taskQueue_clear(qTask* q);
static void  taskQueue_push(qTask* q, Task* task);
static struct task* taskQueue_pop(qTask* q);
static void  taskQueue_deleteAll(qTask* q);

int ThreadPoolInit(struct ThreadPoolManager* t ,int tnum){
    int i = 0;
	if (tnum < 0){
		tnum = 0;
	}

    t->totalThreads = tnum;

	if (taskQueue_init(&t->taskQueue) == -1){
        perror(" Task Queue Init Failed.\n");
		free(t);
		return -1;
	}

	t->threads = (pthread_t*)malloc(tnum * sizeof(pthread_t));
	if (t->threads == NULL){
        perror("Failed allocating memory.\n");
		taskQueue_deleteAll(&t->taskQueue);
		free(t);
		return -1;
	}
	pthread_mutex_init(&(t->plock), NULL);
	pthread_cond_init(&t->pcond, NULL);

	int n;
	for (n=0; n<tnum; n++){
        if (pthread_create(&t->threads[n], NULL, threadPoolCheck, t))
            return -1;
	}
    printf("Done initialize tpool\n");
	return 0;
}

int ThreadPoolInsertTask(ThreadPoolManager* t, void* (*f_p)(void* arg), void* new_arg)
{
	Task* newTask;

	newTask=(struct task*)malloc(sizeof(struct task));
	if (newTask==NULL){
		perror("Could not allocate memory\n");
		return -1;
	}

	newTask->f=f_p;
	newTask->arg=new_arg;

	taskQueue_push(&t->taskQueue, newTask);
    pthread_cond_signal(&t->pcond);

	return 0;
}


void ThreadPoolDestroy(ThreadPoolManager* t){
	if (t == NULL) return ;
    int tnum = t->totalThreads;
	 double TIMEOUT = 2.0;
	 time_t start, end;
	 double tpassed = 0.0;
	 time (&start);
	 while (tpassed < TIMEOUT){
	 	time (&end);
	 	tpassed = difftime(end,start);
	 }

	taskQueue_deleteAll(&t->taskQueue);
	 int n;
     int totalThreads = t->totalThreads;
	 for (n=0; n < totalThreads; n++){
         pthread_cancel(t->threads[n]);
	 }
	free(t->threads);
	
    pthread_mutex_destroy(&t->plock);
    pthread_cond_destroy(&t->pcond);
}

static void* threadPoolCheck(void* threadPool){
	ThreadPoolManager* t = (struct ThreadPoolManager*)threadPool;
	while(1){
        pthread_mutex_lock(&t->plock);
        while(t->taskQueue.lenght ==0){
            pthread_cond_wait(&t->pcond, &t->plock);
        }

        t->numOfCurrThreads++;
        pthread_mutex_unlock(&t->plock);

        void*(*new_f)(void* arg);
        void* new_arg;
        struct task* new_task;

        new_task= taskQueue_pop(&t->taskQueue);
        if(new_task){
            new_f = new_task->f;
            new_arg = new_task->arg;
            new_f(new_arg);
			free(new_task->arg);
            free(new_task);
        }

        pthread_mutex_lock(&t->plock);
        t->numOfCurrThreads--;
        pthread_mutex_unlock(&t->plock);
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
		free(taskQueue_pop(q));
	}

	q->start = NULL;
	q->end = NULL;
	q->lenght = 0;
}

static void taskQueue_push( qTask* q,  Task* task){

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
	Task* new_task = q->start;

	    if (q->lenght == 1){
        q->end = NULL;
        q->start = NULL;
        q->lenght = 0;
    }

    if (q->lenght > 1){
        q->start = new_task->next;
        q->lenght--;
    }

	pthread_mutex_unlock(&q->qlock);
	return new_task;
}

static void taskQueue_deleteAll( qTask* q){
	taskQueue_clear(q);
    pthread_mutex_destroy(&q->qlock);
}