/* user_messenger.c */
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <stdlib.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <arpa/inet.h>

char app_serverIP[9] = "127.0.0.1";
char user_serverIP[9] = "127.0.0.1";
int user_port = 1990;
int temp[1][5];
int tmp_i = 0;
	

void* create_user_server(char ip_add[9]);
void* thread_proc(void *arg);


int main(int argc, char *argv[])
{

	if (argc > 1) {
		user_port = atoi(argv[1]);
	}
	printf("-----------MESSAGES-----------\n");
	printf("	userid: %i\n\n", user_port);
	create_user_server(user_serverIP);
	

}




void* create_user_server(char ip_add[9])
{

	struct sockaddr_in sAddr;
	int listensock;
	int result;
	pthread_t thread_id;
	int x;
	int val;
	int num_thread = 5;

	listensock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	val = 1;
	result = setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (result < 0) {
		perror("server5");
		return 0;
	}
	
	sAddr.sin_family = AF_INET;
	sAddr.sin_port = htons(user_port);
	sAddr.sin_addr.s_addr = inet_addr(app_serverIP);
	//sAddr.sin_addr.s_addr = INADDR_ANY;
	result = bind(listensock, (struct sockaddr *) &sAddr, sizeof(sAddr));
	if (result < 0) {
		perror("server5");
		return 0;
	}
	
	result = listen(listensock, 5);
	
	if (result < 0) {
		perror("server5");
		return 0;
	}
	

	for (x = 0; x < num_thread; x++) {
		result = pthread_create(&thread_id, NULL, thread_proc, (void *)(intptr_t) listensock);
		
		if (result != 0) {
			printf("Could not create thread.\n");
		}

		sched_yield();
	}
	
	pthread_join (thread_id, NULL);
}


void* thread_proc(void *arg)
{
	int listensock,
	sock;
	char buffer[100];
	int nread;
	int ids[10];
	listensock = (intptr_t) arg;
	
	while (1) {
		sock = accept(listensock, NULL, NULL);
		//printf("client connected to child thread %li with pid %li.\n",(intptr_t) pthread_self(),(intptr_t) getpid());
		nread = recv(sock, buffer, 100, 0);
		buffer[nread] = '\0';
		
		int check = 1;
		if (buffer[0] == 'r')
		{
			if(buffer[1] == ':')
			{
				int port = (buffer[3] - '0')*1000 + (buffer[4] - '0')*100 + (buffer[5] - '0')*10 + (buffer[6]- '0'); 					int room = (buffer[2] - '0'); 
				if (port != user_port){

				printf("\n\n----->User-%i entered to the room.\n", port);
				check = 0;
				}
				else{
				printf("\n\n<-- ROOM-%i opened. -->\n", room);
				check = 0;
				}
	
			}
		}

		if (check) 
		{
		printf("%s\n", buffer);
			send(sock, buffer , nread, 0); 
		}
		close(sock);
		//printf("client disconnected from child thread %li with pid %li.\n",(intptr_t) pthread_self(), (intptr_t) getpid());
	}
}


	
	
	
