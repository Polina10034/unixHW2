typedef struct task{
	void*(*f)(void* arg);
	void* arg;
    struct task* next;
} Task;



typedef struct taskQueue{
	pthread_mutex_t qlock;
	Task *start;
	Task *end;
	int lenght;
}qTask;

typedef struct ThreadPoolManager{
	pthread_t* threads;
	pthread_mutex_t plock;
	pthread_cond_t pcond;
	int numOfCurrThreads;
    int totalThreads;
	int* sockets;
	qTask taskQueue;
}ThreadPoolManager;

int ThreadPoolInit(ThreadPoolManager* t, int n);
void ThreadPoolDestroy(ThreadPoolManager* t);
int ThreadPoolInsertTask(ThreadPoolManager* t, void* (*f_p)(void* arg), void* new_arg);


/*bulls and pigs data*/

struct Game{
    int bulls;
    int pigs;
} Game;

struct clientNum{
    pthread_t threadId;
    char serverNum[4];
    char clientNum[4];
} clientNum;