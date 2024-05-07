#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MSG_SIZE 128
#define PATH "/POSIX_server"

int main(void)
{
	struct mq_attr attr;
	mqd_t mqfd;
	char buf[MSG_SIZE];
	unsigned int priority;
	
	attr.mq_maxmsg = 10;										//set maxmsg in mq
	attr.mq_msgsize = MSG_SIZE;									//set msgsize in mq
	attr.mq_flags = 0;
	attr.mq_curmsgs = 0;
	
	if ((mqfd = mq_open(PATH, O_CREAT | O_RDWR, 0666, &attr)) < 0){					//create mq
		perror("mq_open");
		exit(1);
	}
	
	if ((mq_receive(mqfd, &buf[0], MSG_SIZE, &priority)) < 0){					//receive msg from client
		perror("mq_receive");
		exit(1);
	}
	
	if ((strcmp(buf, "Hello!")) == 0){								//if msg == "Hello!"...
		strcpy(buf, "Hi!");
		priority = 1;
		
		if ((mq_send(mqfd, buf, MSG_SIZE, priority)) < 0){					//...send msg back to client
			perror("mq_send");
			exit(1);
		}
		
		printf("Hi!\n");
	}
	
	mq_close (mqfd);										//close mq

	return 0;
}
