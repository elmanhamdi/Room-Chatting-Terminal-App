/* user.c */
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


char app_serverIP[9]= "127.0.0.1";
char user_serverIP[9];
int app_port = 1972;
int user_port = 1990;
int step = 0; 
int room_id = 0;
int num_room = 0;

void child_func(int childnum);
int menu_part();
int show_menu_content(int step);
int send_message(char st[100]);
int send_port(int port, int room);
void* create_user_server(char ip_add[9]);
void* thread_proc(void *arg);
int get_num_room();
int send_signal(char st[10]);



int main(int argc, char *argv[])
{

	if (argc > 1) {
		user_port = atoi(argv[1]);
	}
	else{
		printf("Please enter a valid userid/port");
		exit(0);
	}
	
	num_room = get_num_room();
	
	
	int pid;
	if((pid = fork()) == 0) 
	{
	 	child_func(pid);
	}
	
	else
	{
		char comm[100];
		snprintf(comm, 100, "gnome-terminal -- bash -c \"gcc user_message_part.c -o us -pthread; ./us %i; exec bash\"", user_port);
		int exit_status = system(comm);
	}

	wait(NULL);
	return 0;
}


void child_func(int childnum)
{

	while(1){
		printf("\n-------------------------------------\n");
		while(menu_part());
	}
}

int menu_part()
{
	show_menu_content(step);
	char c;
	printf("\nPlease enter a char: ");
	scanf(" %c", &c);

	
	if (c == 'e'){
		char buffer[7];
		snprintf(buffer, 7, "b:%i", user_port);
		printf("\n%s\n", buffer);
		int response = send_signal(buffer);
		printf("\nGoodbye!!!\n");
		exit(0);
	}
	else if (step)
	{


		if (c == 'm'){
			char message[100];
			printf("\n ->Please enter your message (max 100 character): ");
			//scanf("%s", message);
			
			//to clear buffer
			char temp;
			scanf("%c", &temp);
			
			scanf("%[^\n]", message);
			send_message(message);
		}			
		else if (c == 'b'){
			char buffer[7];
			snprintf(buffer, 7, "b:%i", user_port);
			printf("\n%s\n", buffer);
			int response = send_signal(buffer);
			
			printf("\n main page opened");
			step = 0;
		}
		else{
			printf("\n<YOU ENTERED WRONG>!");
		}

		return 0;
	}
	else
	{
		
		if (c == 'o'){
		
			int in;
			printf("\n ->Please enter group index: ");
			scanf("%d",&in);
			int response = 0;
			response = send_port(user_port, in);
			if (response){
				step = 1;
				room_id = in;
				
				printf("\n RESPONSE SENDED %i", user_port);
				char buffer[19];
				snprintf(buffer,10, "r:%i%i",in, user_port);
				send_signal(buffer);
			}
			else{
				printf("<THE ROOM IS FULL!>");
			}

		}
		else{
			printf("\n<YOU ENTERED WRONG>!");
		}
		return 0;
	}

}

int show_menu_content(int step)
{

	if(step)
	{
		printf(	"\n b = back to main page\n m = create and send a message\n e =exit\n");	
	}
	else
	{
		printf("\n-ROOMS-");
		int i;
		for (i = 0; i < num_room; i ++)
		{
			printf("\n%i - Room %i", i + 1, i + 1);
		}
		printf(	"\n\n o = enter the room\n e =exit\n");
	}
}
		
int send_message(char st[100])
{
	int sock;
	struct sockaddr_in sAddr;
	char buffer[25];
	char buffer2[25];
	memset((void *) &sAddr, 0, sizeof(struct sockaddr_in));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = INADDR_ANY;
	sAddr.sin_port = 0;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr));

	sAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //app_serverIP);
	sAddr.sin_port = htons(app_port + room_id);
	if (connect(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0){
		perror("client");
		return 0;
	}

	snprintf(buffer, 128, "\n User%i: %s", user_port, st);
	send(sock, buffer, strlen(buffer), 0);
				sleep(1);
	recv(sock, buffer2, 25, 0);

	sleep(1);

	close(sock);	
	return 0;	
}

int send_signal(char st[10])
{
	int sock;
	struct sockaddr_in sAddr;
	char buffer[10];

	memset((void *) &sAddr, 0, sizeof(struct sockaddr_in));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = INADDR_ANY;
	sAddr.sin_port = 0;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr));

	sAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //app_serverIP);
	sAddr.sin_port = htons(app_port + room_id);
	if (connect(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0){
		perror("client");
		return 0;
	}

	snprintf(buffer, 10, "%s",st);
	send(sock, buffer, strlen(buffer), 0);
	sleep(1);

	close(sock);	
	return 0;	
}



int send_port(int port, int room)
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
	sAddr.sin_port = htons(1972);
	if (connect(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0){
		perror("client");
		return 0;
	}

	snprintf(buffer, 7, "%c%i%i", c, port, room);
	send(sock, buffer, strlen(buffer), 0);
	sleep(1);
	int nread = recv(sock, buffer, 7, 0);
	buffer[nread] = '\0';
	int response = atoi(buffer);

	close(sock);	
	return response;	
}


int get_num_room()
{
	int sock;
	struct sockaddr_in sAddr;
	char buffer[4];
	char buffer2[25];
	memset((void *) &sAddr, 0, sizeof(struct sockaddr_in));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = INADDR_ANY;
	sAddr.sin_port = 0;
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr));

	sAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); //app_serverIP);
	sAddr.sin_port = htons(1972);
	if (connect(sock, (const struct sockaddr *) &sAddr, sizeof(sAddr)) != 0){
		perror("client");
		return 0;
	}

	snprintf(buffer, 4, "r:");
	send(sock, buffer, strlen(buffer), 0);
				sleep(1);
	recv(sock, buffer2, 25, 0);
	int r_i = atoi(buffer2);

	close(sock);	
	return r_i;	
}



	
	
