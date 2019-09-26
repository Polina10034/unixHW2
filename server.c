#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/select.h>
#include "tpool.h"
#include "tpool.c"

void cleanExit() { exit(0); }
void* gameFunc(void* arg);

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20
int totalThreads;
struct lastGuess *lastGnum;


int main(int argc, char *argv[]){
   
    struct ThreadPoolManager tpManager;
    int reuseaddr = 1;
    fd_set readfds;
    char command[15];

    if( argv[1]  == NULL){
        perror( "Enter number of threads.\n");
        return -1;
    }

    struct sockaddr_in s = {0};
    memset(&s, 0, sizeof(s));
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);

    
    int listenS = socket(AF_INET, SOCK_STREAM, 0);
	if (listenS < 0)
	{
		perror("socket");
		return 1;
	}

    if(setsockopt(listenS, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1){
        perror("setsockopt");
        return 1;
    }
    if (bind(listenS, (struct sockaddr*)&s, sizeof(s)) < 0)
	{
		perror("bind");
		return 1;
	}
	if (listen(listenS, QUEUE_LEN) < 0)
	{
		perror("listen");
		return 1;
	}

    totalThreads = strtol(argv[1], NULL, 10);
    lastGnum = (struct lastGuess*)malloc(totalThreads * sizeof(struct lastGuess));
    if(lastGnum == NULL){
        perror("Faild allocating lastGnum.\n");
        return -1;
    }
    else {
            printf("allocating lastgnum done.\n");
        }

    for(int i=0; i<totalThreads; i++){
        lastGnum[i].threadId = 0;
       // printf("lastGus num = 0\n" );
    }

    printf("Making threadPool with %d threads\n" ,totalThreads);
    if(ThreadPoolInit(&tpManager, totalThreads) == -1){
        printf("Faild threadPoolInit(). \n");
        return -1;
    }
    printf("Please enter one command: list or quit.\n");
    while(1){
        struct sockaddr_in addr;
        socklen_t size = sizeof(struct sockaddr_in);
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(listenS, &readfds);

        int maxfd = listenS;

        if(select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0){
            perror("select");
            return -1;        
         }

    /* commands */
        
        if(FD_ISSET(STDIN_FILENO, &readfds)){
           
            fgets(command,15,stdin);
            printf("command: %s\n", command);
            if(strcmp(command, "list\n") == 0){
                printf("Listing.\n");
                for(int i=0; i< totalThreads; i++){
                    if(lastGnum[i].threadId == 0)
                        break;
                    printf("Game %d - Number %s, Last Guess: %s\n",i,lastGnum[i].sNum,lastGnum[i].guess);
               }

            }
            else if( strcmp(command, "quit\n") == 0){
                printf("Quiting...\n");
                ThreadPoolDestroy(&tpManager);
                for(int i = 0; i < maxfd; i++){
                    close(i);
                }
             
            }

            else {
                printf("Renter the command:\n");
            }
            continue;

        }

printf("Listening for connection.\n");
        int newsocket = accept(listenS, (struct sockaddr*)&addr, &size);
        if(newsocket == -1){
            perror("accept");
            return -1;
        }
        
        printf("Connection accepted from %s:%d.\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        struct task* new_task ;
        new_task = (struct task*)malloc(sizeof( struct task));
        /*int* newsocket_p = (int*)malloc(sizeof(int));*/

        int* nsocketPtr = (int*)malloc(sizeof(int));
        if (nsocketPtr  == NULL){
            perror("Falid allocating new_task socket ptr.\n");
            return -1;
        }
        else {
            printf("allocating nsockPtr done.\n");
        }  
        *nsocketPtr =  newsocket;
        new_task->arg = (void*)nsocketPtr;
        new_task->f = gameFunc;
        ThreadPoolInsertTask(&tpManager, new_task);
        }

         free(lastGnum);
         close(listenS);
         printf("Exiting.\n");
         return 0;
    }

    void* gameFunc(void* arg){
        int newsocket = *(int*)arg;
        char serverNum[4];
        char clientNum[4];
        struct Game gameResult ;

        gameResult.bulls = 0;
        gameResult.pigs = 0;

        char randNum = 0;
        srand(time(NULL));

        printf("Game # %d starting\n", (int)pthread_self());
        while(1){
        /*creating random number without repeat*/
        
        for(int i =0 ; i < 4 ; i++){
            randNum = '1' + random() % 9;
           /* printf("Rand NUm is: %c.\n", randNum);*/
            for(int j=0; j < 4 ; j++){
                if (serverNum[j] == randNum ){
                   randNum = '1' + random() % 9;
                   j=0;
                }
            }
            serverNum[i]= randNum;
        }
            serverNum[4]= '\0';
            printf("Server number: %s.\n",serverNum);
        }

        while(1){
            if (recv(newsocket, &clientNum, sizeof(clientNum), 0) < 0)
			{
				perror("recv");
				return NULL;
			}

            for( int i=0 ; i < totalThreads ; i++){
                if( lastGnum[i].threadId == 0){
                    lastGnum[i].threadId = pthread_self();
                    strcpy(lastGnum[i].guess, clientNum);
                            printf("Client num in gues: %s.\n",lastGnum[i].guess);
                    strcpy(lastGnum[i].sNum, serverNum);
                          printf("Server num in gues: %s.\n",lastGnum[i].sNum);

                  }
                else if(lastGnum[i].threadId == pthread_self()){
                    strcpy(lastGnum[i].guess , clientNum);
                }
            }
              printf("Client number: %s.\n",clientNum);
              sleep(1);
            
//// until here everything  is working /////
            /*compering numbers*/

            for(int i = 0 ;i< 4 ; i++){
                for(int j = 0 ; j < 4 ; j++){
                    if (i == j && serverNum[i] == clientNum[j])
                        gameResult.bulls++;
                    else if (i != j && serverNum[i] == clientNum[j])
                        gameResult.pigs++;
                }
            }
        }
        if((write(newsocket, &gameResult.bulls, sizeof(int))) < 0){
                perror("write().\n");
                return NULL;
        }
        if((write(newsocket, &gameResult.pigs, sizeof(int))) < 0){
                perror("write().\n");
                return NULL;
        }
        if(gameResult.bulls == 4){
            printf("Correct answer! You Won\n");
        }

        printf("Game # %d finished.\n", (int)pthread_self());
        close(newsocket);

    }