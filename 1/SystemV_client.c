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
	
	if ((key = ftok(msg_path, 10)) < 0){									//generate key with the same paramets as on server
		perror("ftok");
		exit(1);
	}
	
	if ((msqid = msgget(key, 0)) < 0){									//connect to mq
		perror("msqid");
		exit(1);
	}
	
	message.mtype = 1;
	strcpy(message.mtext, "Hello!");		
	
	if ((msgsnd(msqid, &message, msg_size, IPC_NOWAIT)) < 0){						//send msg to server
		perror("msgsnd");
		exit(1);
	}
	
	if ((msgrcv(msqid, &message, msg_size, 1, 0)) < 0){							//receive a response msg
		perror("msgrcv");
		exit(1);
	}
	
	if ((strcmp(message.mtext, "Hi!")) == 0){							
		printf("Hello!\n");
	}
	
	return 0;
}
