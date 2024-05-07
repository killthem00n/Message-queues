#include <ncurses.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXCLIENTS 5
#define NCKNMSIZE 20
#define MSGSIZE 128
#define PATH "SystemV_server"

struct msgbuf {															//msg struct
	long mtype;
	char mtext[MSGSIZE];
	char nick[NCKNMSIZE];
	int id;
};

int mq_income;															//income mq fd
int mq_chat;															//chat mq fd
	
void *chat_sender(void *args)
{
	struct msgbuf chat_receive;
	int ids[MAXCLIENTS] = { 0 };												//list of connected clients
	int symbol_counter;
	int mismatch_counter = 0;
	
	while (1){
		symbol_counter = msgrcv(mq_income, &chat_receive, sizeof(struct msgbuf), 1, IPC_NOWAIT);			//receive msg from client
		
		if (symbol_counter > 0){											
			for (int i = 0; i < MAXCLIENTS; i++){
				if (ids[i] != chat_receive.id){									//checking whether the client is in the list of connected ones
					mismatch_counter++;
				}
			}
			
			if (mismatch_counter == 5){										//if he's not there, add him to to the first free slot
				for (int i = 0; i < MAXCLIENTS; i++){									
					if (ids[i] == 0){
						ids[i] = chat_receive.id;							
						break;
					}
				}
			}
			
			mismatch_counter = 0;
			
			if (strcmp(chat_receive.mtext, "left111") == 0){							//if client msg == "left111", exclude him from the list of connected
				for (int i = 0; i < MAXCLIENTS; i++){
					if (ids[i] == chat_receive.id){
						ids[i] = 0;
						break;
					}
				}
			}
		
			for (int i = 0; i < MAXCLIENTS; i++){									//send msg to each client using his id as the priority of the message
				if (ids[i] != 0){
					if (ids[i] != chat_receive.id){
						chat_receive.mtype = ids[i];
						if ((msgsnd(mq_chat, &chat_receive, sizeof(struct msgbuf), IPC_NOWAIT)) < 0){
							perror("msgsnd: chat_sender");
							exit(1);
						}
					}
				}
			}
		}
		
		sleep (0.05);
	}
}

int main(void)
{
	pthread_t THREAD;
	int *status;
	key_t key_income;
	key_t key_chat;
	
	if ((key_income = ftok(PATH, 33)) < 0){									//generate key				
		perror("ftok: income");
		exit(1);
	}
	
	if ((mq_income = msgget(key_income, IPC_CREAT | 0666)) < 0){						//create mq for all incoming messages
		perror("msgget: income");
		exit(1);
	}
	
	if ((key_chat = ftok(PATH, 55)) < 0){									//generate key		
		perror("ftok: chat");
		exit(1);
	}
	
	if ((mq_chat = msgget(key_chat, IPC_CREAT | 0666)) < 0){						//create mq for chat
		perror("msgget: chat");
		exit(1);
	}

	pthread_create(&THREAD, NULL, chat_sender, NULL);							//create thread that will send messages to each client
	
	pthread_join(THREAD, (void **)&status);									//terminate thread
	
	if ((msgctl(mq_income, IPC_RMID, NULL)) < 0){								//remove income mq
		perror("msgctl: income");
		exit(1);
	}
	
	if ((msgctl(mq_chat, IPC_RMID, NULL)) < 0){								//remove chat mq
		perror("msgctl: chat");
		exit(1);
	}
	
	return 0;
}
