/* app_server.c */
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
#include <syslog.h>
#include <sys/stat.h>
#include <pwd.h>


char app_serverIP[9]= "127.0.0.1";
void* thread_proc(void *arg);
void* thread_proc2(void *arg);
void* room_server_creater(int port);
void* main_menu_server_create();
int add_port_to_room(int port, int room);
int num_room = 3;


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

	
	
		
	int x;
	int pid;
	for (x = 0; x < num_room; x++) {
		
		if((pid = fork()) == 0) 
		{
			sleep(1*(x + 1));
			char buffer[100];
			snprintf(buffer, 100, "gnome-terminal -- bash -c \"./r %i;\"", x + 1);
		 	//snprintf(buffer, 100, "gnome-terminal -- bash -c \"gcc room.c  -o r  -pthread ; ./r %i;\"", x + 1);
		 	//snprintf(buffer, 100, "gnome-terminal -- bash -c \"./r %i; exec bash\"", x + 1);
		 	int exit_status = system(buffer);
		 	
		 	exit(0);
		}
		
	}
	daemonize();
	main_menu_server_create();
	

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
		printf("%s\n", buffer);
		send(sock, buffer , nread, 0); 
		close(sock);
	}
}
	
void* thread_proc2(void *arg)
{	
	int listensock,
	sock;
	char buffer[6];
	int nread;
	int ids[10];
	listensock = (intptr_t) arg;
	
	while (1) {
		sock = accept(listensock, NULL, NULL);
		//printf("MAIN:client connected to child thread %li with pid %li.\n",(intptr_t) pthread_self(),(intptr_t) getpid());
		nread = recv(sock, buffer, 6, 0);
		buffer[nread] = '\0';
		int port = (buffer[1] - '0')*1000 + (buffer[2] - '0')*100 + (buffer[3] - '0')*10 + (buffer[4]- '0'); 
		int room = buffer[5] - '0';
		
		int check = 1;
		if( buffer[0] == 'o')
		{
			int response = add_port_to_room(port, room);
			snprintf(buffer, 2, "%i",response);
			send(sock, buffer ,2, 0); 
			check = 0;
			
		}
		
		else if (buffer[0] == 'r')
		{
			if(buffer[1] == ':')
			{
				snprintf(buffer,6, "%i", num_room);
				send(sock, buffer, 6, 0); 
				check = 0;
			}
		}
		if (check) send(sock, buffer , nread, 0); 
		close(sock);
		//printf("MAIN: client disconnected from child thread %li with pid %li.\n",(intptr_t) pthread_self(), (intptr_t) getpid());
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

	
void* main_menu_server_create()
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
	sAddr.sin_port = htons(1972);
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
		result = pthread_create(&thread_id, NULL, thread_proc2, (void *)(intptr_t) listensock);
		
		if (result != 0) {
			printf("Could not create thread.\n");
		}

		sched_yield();
	}
	
	pthread_join (thread_id, NULL);
}


int add_port_to_room(int port, int room)
{
	char c = 'o';
	int sock;
	struct sockaddr_in sAddr;
	char buffer[7];
	char buffer2[7];
	memset((void *) &sAddr, 0, sizeof(struct sockaddr_in));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = INADDR_ANY;
	sAddr.sin_port = 0;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr));

	sAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //app_serverIP);
	sAddr.sin_port = htons(1972 + room);
	if (connect(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0){
		perror("client");
		return 0;
	}

	snprintf(buffer, 7, "a:%i", port);
	send(sock, buffer, strlen(buffer), 0);
				sleep(1);
	
	int nread;
	nread = recv(sock, buffer, 7, 0);
	buffer[nread] = '\0';
	int result = atoi(buffer);

	close(sock);	
	return result;	
}



	
	
	
