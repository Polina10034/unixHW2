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
#include "tpool.h";

void cleanExit() { exit(0); }

#define PORT 0x0da2
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20



int main(int argc, char *argv[]){
   
    int randNum;
    srand(time(NULL));
    randNum= rand();
    struct lastGuess* lastGnum;

    if( argv[1]  == NULL){
        perror( "Enter number of threads.\n");
        return -1;
    }

    int toatlThreads = strtol(argv[1], NULL, 10);

	
    struct sockaddr_in s = {0};
	s.sin_family = AF_INET;
	s.sin_port = htons(PORT);
	s.sin_addr.s_addr = htonl(IP_ADDR);

    int listenS = socket(AF_INET, SOCK_STREAM, 0);
	if (listenS < 0)
	{
		perror("socket");
		return -1;
	}

    if (bind(listenS, (struct sockaddr*)&s, sizeof(s)) < 0)
	{
		perror("bind");
		return -1;
	}
	if (listen(listenS, QUEUE_LEN) < 0)
	{
		perror("listen");
		return -1;
	}
}