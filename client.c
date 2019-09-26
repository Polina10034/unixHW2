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
#include "tpool.c"

#define PORT 0x0da2
#define IP_ADDR 0x7f000001

int clientGameFunc(int sfd );

int main( int argc, char** argv){

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("socket() client faild\n");
        return -1;
    }
    
    struct sockaddr_in s = {0};
    s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);
	if (connect(sock, (struct sockaddr *)&s, sizeof(s)) < 0)
	{
		perror("connect");
		return 1;
	}
	printf("Successfully connected.\n");    
    
    int n = clientGameFunc(sock);
    close(sock);
    return 0;

   
}

int clientGameFunc(int sfd ){
    char clientNum[4];
    int bulls;
    int pigs;
    int returndNum;
    fd_set readfds;

    printf("Starting Game \n");

    while(1){
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sfd, &readfds);

        int maxfd = sfd;

        printf("Please Guess 4 digits between 1-9:\n");
        
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0){
            perror("select");
            return -1;
        }

        //cheking STDIN//
        if(FD_ISSET(STDIN_FILENO, &readfds)){
            scanf("%s", clientNum);
            if(send(sfd, clientNum, sizeof(clientNum), 0) < 0){
                perror("faild sending client num...\n");
                return -1;
            }
         }
         else
         {
             if((returndNum = read(sfd, &bulls, sizeof(int)) <= 0)){
                perror("Conection CLOSED.\n");
                close(sfd);
                return -1;                
             }

             if((returndNum = read(sfd, &pigs, sizeof(int)) <= 0)){
                perror("Conection CLOSED.\n");
                close(sfd);
                return -1;                
             }

             printf("Game Results: Bools: %d, Pigs: %d \n", bulls, pigs);

             if(bulls == 4){
                 printf("Correct Guess!\n");
                 printf("Game is Over.\n");
                 close(sfd);
                 return 1;
             }
             
         }
         
    }
   
}