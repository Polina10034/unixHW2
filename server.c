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
#include <pthread.h>
#include "tpool.h"
#include <stdlib.h>
#include <unistd.h>

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20

void handle(int newsocket, int filefd, int req_type);
int listenToStdin(void *tpManager);
void *gameFunc(void *threadPool);
void *getAdminInput(void *tpManager);

struct clientNum *lastGnum;
int totalThreads;

int main(int argc, char *argv[])
{
    int filefd;
    struct sockaddr_in s, *res;
    int reuseaddr = 1; /* True */
    int getConnections = 1;
    fd_set readfds;
    char command[15];

    if (argv[1] == NULL)
    {
        printf("Error:please enter number of threads...\n");
        return -1;
    }
    /* convert num of threads to int */
    totalThreads = strtol(argv[1], NULL, 10);

    
    memset(&s, 0, sizeof(s));
    s.sin_family = AF_INET;
    s.sin_port = ntohs(PORT);
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

    
    lastGnum = malloc(totalThreads * sizeof(struct clientNum));
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
    ThreadPoolManager tpManager;
    if(ThreadPoolInit(&tpManager, totalThreads) == -1){
        printf("Faild threadPoolInit(). \n");
        return -1;
    }
    printf("Please enter one command: list, quit or kick.\n");
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
            if(strcmp(command, "list\n") == 0){ //list all games
                printf("Listing.\n");
                for(int i=0; i< totalThreads; i++){
                    if(lastGnum[i].threadId == 0)
                        break;
                    printf("Game %d - Number %s, Last Guess: %s\n",i,lastGnum[i].serverNum, lastGnum[i].clientNum);
                }

            }
            else if( strcmp(command, "quit\n") == 0){ //close server
                printf("Quiting...\n");
                ThreadPoolDestroy(&tpManager);
                for(int i = 3; i < maxfd; i++){
                    close(i);
                }
                break;
            }
            else if( strcmp(command, "kick\n") == 0){//kick player for server
                printf("Please enter game number:\n");
                int kickN = 0;
                scanf("%d", &kickN);
                pthread_cancel(lastGnum[kickN].threadId);
                break;
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

        if (newsocket > maxfd)
        {
            maxfd = newsocket;
        }
        printf("Connection accepted from %s:%d.\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        
        int* nsocketPtr = (int*)malloc(sizeof(int));
        if (nsocketPtr  == NULL){
            perror("Falid allocating new_task socket ptr.\n");
            return -1;
        }
        else {
            printf("allocating nsockPtr done.\n");
        }  
        *nsocketPtr =  newsocket;
        ThreadPoolInsertTask(&tpManager, gameFunc, (void *)nsocketPtr);
    }
    free(lastGnum);
    close(listenS);
    printf("Exiting.\n");
    return 0;
}

void* gameFunc(void* arg){
    int newsocket = *(int*)arg;
    int i = 0, j = 0;
    int grecv;
    char serverNum[4];
    char clientNum[4];
    struct Game gameResult ;

    serverNum[4] = '\0'; //to be sure its 0
    clientNum[4] = '\0';
    gameResult.bulls = 0;
    gameResult.pigs = 0;

    srand(time(NULL));

    printf("Game # %d starting\n", (int)pthread_self());
    while (1)
    {
        for (i = 0; i < 4; i++)
        {
            /*creating random number without repeat*/
            serverNum[i] = '1' + (random() % 9);
        }

        if ((serverNum[0] != serverNum[1]) &&
            (serverNum[0] != serverNum[2]) && 
            (serverNum[0] != serverNum[3]) && 
            (serverNum[1] != serverNum[2]) && 
            (serverNum[1] != serverNum[3]) && 
            (serverNum[2] != serverNum[3]))
        {
            break;
        }
    }
        printf("Server number: %s.\n",serverNum);

    while (1){
        if (grecv = recv(newsocket, clientNum, sizeof(clientNum), 0) <= 0)
        {
			perror("recv");
            break;
        }

        for( int i=0 ; i < totalThreads ; i++){
            if( lastGnum[i].threadId == 0){
                lastGnum[i].threadId = pthread_self();
                strcpy(lastGnum[i].clientNum, clientNum);
                strcpy(lastGnum[i].serverNum, serverNum);
                break;
            }
            else if(lastGnum[i].threadId == pthread_self()){
                strcpy(lastGnum[i].clientNum, clientNum);
                break;
            }
        }
        printf("Client number: %s.\n",clientNum);
        sleep(1);

        /*compering numbers*/
        for(i = 0; i< 4 ; i++){
            for(j = 0; j < 4 ; j++){
                if ((i == j) && (serverNum[i] == clientNum[j]))
                    gameResult.bulls++;
                else if (serverNum[j] == clientNum[i]){
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
            break;
        }

        grecv = 0; //reset values
        gameResult.bulls = 0;
        gameResult.pigs = 0;
    }
    printf("Game # %d finished.\n", (int)pthread_self());
    close(newsocket);
}