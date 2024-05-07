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
	
	if ((mqfd = mq_open(PATH, O_RDWR)) < 0){								//connect to mq
		perror("mq_open");
		exit(1);
	}
	
	strcpy(buf, "Hello!");
	priority = 1;
	
	if ((mq_send(mqfd, buf, MSG_SIZE, priority)) < 0){							//send msg to server
		perror("mq_send");	
		exit(1);
	}
	
	if ((mq_receive(mqfd, &buf[0], MSG_SIZE, &priority)) < 0){						//receive a response msg
		perror("mq_receive");
		exit(1);
	}
	
	if ((strcmp(buf, "Hi!")) == 0){
		printf("Hello!\n");
	}
	
	mq_close (mqfd);

	return 0;
}
