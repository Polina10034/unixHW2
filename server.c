#include <string.h>
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

void cleanExit() { exit(0); }

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20



int main(int argc, char *argv[]){
   
    int randNum;
    srand(time(NULL));
    randNum= rand();
    struct lastGuess* lastGnum;
    struct ThreadPoolManager tpManager;
    int reuseaddr = 1;
    fd_set readfds;
    char command[15];

    if( argv[1]  == NULL){
        perror( "Enter number of threads.\n");
        return -1;
    }

    struct sockaddr_in s = {0};
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

    int totalThreads = strtol(argv[1], NULL, 10);
    lastGnum = malloc(totalThreads * sizeof(struct lastGuess));
    if(lastGnum == NULL){
        perror("Faild allocating lastGnum.\n");
        return -1;
    }

    /*for(int i=0; i<totalThreads; i++){

    }*/

    if(ThreadPoolInit(&tpManager, totalThreads) == -1){
        printf("Faild threadPoolInit(). \n");
        return -1;
    }

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
        printf("Please enter one command: list or quit.");
        if(FD_ISSER(STDIN_FILENO, &readfds)){
           
            fgets(command, 15, stdin);
            
            if(strcmp(command, "list\n") == 0){
                printf("Listing.\n");
                for(int i=0; i< totalThreads; i++){
                    if(lastGnum[i].threadId == 0)
                        break;
                    printf("Game %d - Number %s, Last Guess: %s\n", i,lastGnum[i].sNum,lastGnum[i].guess);
               }

            }
            else if( strcmp(command, "quit\n") == 0){
                printf("Quiting...\n");
                ThreadPoolDestroy(&tpManager);
                for(int i = 0; i < maxfd; i++){
                    close(i);
                }
                break;
            }

            else {
                printf("Renter the command:\n");
            }

        }
        


}