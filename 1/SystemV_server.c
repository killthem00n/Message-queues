#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define msg_size 128
#define msg_path "SystemV_server"

struct msgbuf {
	long mtype;
	char mtext[msg_size];
};

int main(void)
{
	struct msgbuf message;
	key_t key;
	int msqid;
	
	if ((key = ftok(msg_path, 10)) < 0){									//generate key
		perror("ftok");
		exit(1);
	}
	
	if ((msqid = msgget(key, IPC_CREAT | 0666)) < 0){							//create mq with key
		perror("msqid");
		exit(1);
	}
	
	if ((msgrcv(msqid, &message, msg_size, 1, 0)) < 0){							//receive msg from client
		perror("msgrcv");
		exit(1);
	}
		
	if ((strcmp(message.mtext, "Hello!")) == 0){								//if msg == "Hello!"...
		strcpy(message.mtext, "Hi!");									
		message.mtype = 1;
			
		if ((msgsnd(msqid, &message, msg_size, IPC_NOWAIT)) < 0){					//...send msg back to client
			perror("msgsnd");
			exit(1);
		}
			
		printf("Hi!\n");
	}
	
	if ((msgctl(msqid, IPC_RMID, NULL)) < 0){								//delete mq
		perror("msgctl");
		exit(1);
	}
	
	return 0;
}
