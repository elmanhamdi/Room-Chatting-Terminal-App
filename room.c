/* server5.c */
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
/* room.c */
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
#include <syslog.h>
#include <sys/stat.h>
#include <pwd.h>

char app_serverIP[9]= "127.0.0.1";
void* thread_proc(void *arg);
void* thread_proc2(void *arg);
void* create_new_group(void *arg);
void* room_server_creater(int port);
int send_message(char st[100]);

int num_room;
int MAX_USER = 2;
int room_users[2];
int tmp_i = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


//Deamonizing part
int daemonize()
{
	pid_t pid;
	long n_desc;
	int i;
	
	if ((pid = fork()) != 0) {
		exit(0);
	}
	
	setsid();
	
	if ((pid = fork()) != 0) {
		exit(0);
	}
	
	chdir("/var/log/");
	umask(0);
	
	n_desc = sysconf(_SC_OPEN_MAX);
	for (i = 0; i < n_desc; i++) {
	close(i);
	}
	return 1;
}


int main(int argc, char *argv[])
{
	daemonize();
	int room = 0;
	if (argc > 1) {
		room = atoi(argv[1]);
	}
	else {printf("\nRoom ID is needed\n"); exit(0);}


	room_server_creater(room);
	

}

void* thread_proc(void *arg)
{	
	int listensock,
	sock;
	char buffer[25];
	int nread;
	int ids[10];
	listensock = (intptr_t) arg;
	
	while (1) {
		sock = accept(listensock, NULL, NULL);
		nread = recv(sock, buffer, 25, 0);
		buffer[nread] = '\0';
		printf("here:%s\n", buffer);
		
		int check = 1;
		if (buffer[0] == 'a')
		{
			if(buffer[1] == ':')
			{
				pthread_mutex_lock(&mutex);
				check = 0;
				if (tmp_i < MAX_USER){
					int port = (buffer[2] - '0')*1000 + (buffer[3] - '0')*100 + (buffer[4] - '0')*10 + (buffer[5] - '0'); 
					room_users[tmp_i] = port;
					tmp_i ++;
					check = 0;
					send(sock, "1" , 2, 0); 
				}
				else{
					send(sock, "0" , 2, 0); 
				}
				pthread_mutex_unlock(&mutex);
			}
		}
		else if (buffer[0] == 'b')
		{
			if(buffer[1] == ':')
			{
				pthread_mutex_lock(&mutex);
				check = 0;
				int port = (buffer[2] - '0')*1000 + (buffer[3] - '0')*100 + (buffer[4] - '0')*10 + (buffer[5] - '0'); 
				int i;
				for (i = 0; i < tmp_i; i ++)
				{
					if(port == room_users[i]){
						
						char buffer2[30];
						snprintf(buffer2, 30, "\n----->User%i left the room.\n", port);
						send_message(buffer2);

						int j;
						for (j = i; j < tmp_i; j ++){
							room_users[j] = room_users[j + 1];
							
						}
						tmp_i --;
						
					}			
				}
				pthread_mutex_unlock(&mutex);

			}
		}

		if (check) 
		{
		send_message(buffer);
		}

		
		close(sock);
	}
}
		

void* room_server_creater(int port)
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
	sAddr.sin_port = htons(1972 + port);
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


int send_message(char st[100])
{
	int i;
	for (i = 0; i < tmp_i; i ++)
	{	
		int sock;
		struct sockaddr_in sAddr;
		char buffer[100];
		memset((void *) &sAddr, 0, sizeof(struct sockaddr_in));
		sAddr.sin_family = AF_INET;
		sAddr.sin_addr.s_addr = INADDR_ANY;
		sAddr.sin_port = 0;
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		bind(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr));

		sAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //app_serverIP);
	
		sAddr.sin_port = htons(room_users[i]);
		if (connect(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0){
			perror("client");
			return 0;
		}
		
		snprintf(buffer, 100, "%s", st);
		send(sock, buffer, strlen(buffer), 0);
		close(sock);	
	}
	sleep(1);
	return 0;	
}


	
	
	
