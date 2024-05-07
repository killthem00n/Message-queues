#include <ncurses.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define NCKNMSIZE 20
#define MSGSIZE 128
#define PATH "SystemV_server"

struct msgbuf {														//msg struct
	long mtype;
	char mtext[MSGSIZE];
	char nick[NCKNMSIZE];
	int id;
};

int my_id;														//unique client id
int mq_outcome;														//outcome mq fd
int mq_chat;														//chat mq fd

void *chat_receiver(void *args)
{
	struct msgbuf chat_receive;
	int counter;
	
	while (1){
		counter = msgrcv(mq_chat, &chat_receive, sizeof(struct msgbuf), my_id, IPC_NOWAIT);			//receive msg from server
		
		if (counter > 0){
			if (strcmp(chat_receive.mtext, "left111") == 0){						//if msg == "left111", write that client has disconnected
				printf("%s has left the chat\n", chat_receive.nick);
				continue;
			}
			
			if (strcmp(chat_receive.mtext, "joined111") == 0){						//if msg == "joined111", write that client has connected
				printf("%s has joined the chat\n", chat_receive.nick);
				continue;
			}
			
			printf("%s: %s", chat_receive.nick, chat_receive.mtext);					//else print regular msg
		}
		
		sleep(0.5);
	}
}

void *chat_sender(void *args)
{
	struct msgbuf chat_send;
	
	printf("Enter your nickname: ");						
	fgets(chat_send.nick, NCKNMSIZE, stdin);									//client chooses a nickname
	chat_send.nick[strcspn(chat_send.nick, "\n")] = 0;	
	chat_send.id = my_id;												//unique client id for server		
	chat_send.mtype = 1;
	strncpy(chat_send.mtext, "joined111", 9);									//"joined111" that server adds client  to the list of connected
	
	if ((msgsnd(mq_outcome, &chat_send, sizeof(struct msgbuf), IPC_NOWAIT)) < 0){					//send all client info to server
		perror("msgsnd: chat_sender (join message)");
		exit(1);
	}
	
	printf("\n");
	
	while (1){
		fgets(chat_send.mtext, MSGSIZE, stdin);									//thread is waiting for a message
		
		if (strcmp(chat_send.mtext, "/exit\n") == 0){								//if client enter "/exit", the program ends after sending a notification to...
			memset(&chat_send.mtext, 0, MSGSIZE);								//...the server to remove client from the list of connected
			strcat(chat_send.mtext, "left111");
			if ((msgsnd(mq_outcome, &chat_send, sizeof(struct msgbuf), IPC_NOWAIT)) < 0){
				perror("msgsnd: chat_sender (exit message)");
				exit(1);
			}
			
			exit(1);
		}
		
		if ((msgsnd(mq_outcome, &chat_send, sizeof(struct msgbuf), IPC_NOWAIT)) < 0){				//send regular message to server
			perror("msgsnd: chat_sender (default message)");
			exit(1);
		}
		
		sleep(0.5);
	}
}

int main(void)
{
	srand(time(NULL));
	pthread_t THREADS[2];
	int *status;
	key_t key_outcome;
	key_t key_chat;
	
	if ((key_outcome = ftok(PATH, 33)) < 0){									//generate key	
		perror("ftok: outcome");
		exit(1);
	}
	
	if ((mq_outcome = msgget(key_outcome, 0)) < 0){									//create mq for all outcoming messages
		perror("msgget: outcome");
		exit(1);
	}
	
	if ((key_chat = ftok(PATH, 55)) < 0){										//generate key	
		perror("ftok: nickname");
		exit(1);
	}
	
	if ((mq_chat = msgget(key_chat, 0)) < 0){									//create mq for chat
		perror("msgget: nickname");
		exit(1);
	}
	
	my_id = rand()%10000;												//generate unique id for using it as a message priority
	
	pthread_create(&THREADS[0], NULL, chat_receiver, NULL);								//create thread for receiving messages
	pthread_create(&THREADS[1], NULL, chat_sender, NULL);								//create thread for sending messages
	
	for (int i = 0; i < 2; i++){											//terminate threads
		pthread_join(THREADS[i], (void **)&status);
	}
	
	return 0;
}
