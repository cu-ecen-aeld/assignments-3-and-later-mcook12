// Application to open stream socket and append recieved data to 
///var/tmp/aesdsocketdata, then return content to the client
//Author: Michael Cook

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define DATA_PATH "/var/tmp/aesdsocketdata"

int sockfd;
int newfd;
int fd;

pthread_mutex_t s_mutex;

struct thread_data {
	pthread_t thread_id;
	int socket_id;
	bool isDone;
	SLIST_ENTRY(thread_data) entries;
};

SLIST_HEAD(slisthead, thread_data) head;

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void terminate(int signum)
{
	syslog(LOG_INFO, "Caught signal, exiting");
	closelog();
	close(newfd);
	close(sockfd);
	close(fd);

	remove(DATA_PATH);
	exit(0);
}

void* socket_handler(void* arg){
	struct thread_data *t_data = (struct thread_data*) arg;
	int socket_id = t_data->socket_id;
	ssize_t stor;
	char buffer[BUF_SIZE];
        while ((stor = recv(socket_id, buffer, BUF_SIZE -1, 0)) > 0){
		buffer[stor] = '\0';

		pthread_mutex_lock(&s_mutex);
		if ((fd = open(DATA_PATH, O_CREAT | O_WRONLY | O_APPEND, 0644)) >= 0){
			write(fd, buffer, stor);
			close(fd);
		}
		pthread_mutex_unlock(&s_mutex);
		if (strchr(buffer, '\n')) break;
	}

	pthread_mutex_lock(&s_mutex);
	int fd = open(DATA_PATH, O_RDONLY);
	if (fd >=0){
		while ((stor = read(fd, buffer, BUF_SIZE)) >0) {
			send(socket_id, buffer, stor, 0);
		}
		close(fd);
	}
	pthread_mutex_unlock(&s_mutex);

	close(socket_id);
	return NULL;
}

void* timestamp_handler(void* arg) {
	for(;;) {
		sleep(10);
		pthread_mutex_lock(&s_mutex);
		if ((fd = open(DATA_PATH, O_CREAT | O_WRONLY | O_APPEND, 0644)) >=0) {
			time_t t_now = time(NULL);
			struct tm *tm_data = localtime(&t_now);
			char buffer_t[100];
			strftime(buffer_t, sizeof(buffer_t), "timestamp:&a, %d %b %Y %H:%M:%S %z\n", tm_data);
			write(fd, buffer_t, strlen(buffer_t));
			close(fd);
		}
		pthread_mutex_unlock(&s_mutex);
	}
	return NULL;
}

int main(int argc, char *argv[]){
	char s[INET6_ADDRSTRLEN];
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	bool daemonize = false;
	int opt;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;
	
	pthread_mutex_init(&s_mutex, NULL);
	
	while ((opt = getopt(argc, argv, "d")) != -1) {
		switch (opt) {
			case 'd':
				daemonize = true;
				break;
			default:
				//usage error
				return -1;
		}
	}

	openlog (NULL, 0, LOG_USER);

	if ((getaddrinfo(NULL, "9000", &hints, &servinfo)) == -1) {
		//error
		return -1;
	}
	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		//error
		return -1;
	}
	int sockopt = 1;
	if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt))) == -1) {
			//error
			return -1;
	}
	if ((bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
		//error
		return -1;
	}

	freeaddrinfo(servinfo);

	if(daemonize) {
		pid_t pid;
		pid = fork();
		if (pid == -1){
			//error
			exit(EXIT_FAILURE);
		}
		if (pid > 0) {
			exit(EXIT_SUCCESS);
		}
		if (setsid() == -1) {
			//error
			exit(EXIT_FAILURE);
		}
		if (chdir("/") == -1) {
			//error
			exit(EXIT_FAILURE);
		}
	}

	if ((listen(sockfd, 10)) == -1) {
		//error
		close(sockfd);
		return -1;
	}
	
	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);

	pthread_t timestamp_thread;
	pthread_create(&timestamp_thread, NULL, timestamp_handler, NULL);

	printf("\n test1");
	for(;;) {
		if ((newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) == -1) {
			//error
			return -1;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		syslog (LOG_INFO, "Accepted Connection from %s", s);

		struct thread_data *t_data = malloc(sizeof(struct thread_data));
		t_data->socket_id = newfd;
		SLIST_INSERT_HEAD(&head, t_data, entries);
		pthread_create(&t_data->thread_id, NULL, socket_handler, t_data);
	}

	struct thread_data *t_data;
	while(!SLIST_EMPTY(&head)) {
		t_data = SLIST_FIRST(&head);
		pthread_join(t_data->thread_id, NULL);
		SLIST_REMOVE_HEAD(&head, entries);
		free(t_data);
	}
	close(sockfd);
	//remove(DATA_PATH);

	//	if (lseek(fd, 0, SEEK_END) == -1) {
	//		//error
	//		return -1;
	//	}
	//	for (;;) {
	//		if((stor = recv(newfd, buf, sizeof(buf), 0)) == -1) {
	//			//error
	//			return -1;
	//		}
	//		if((stor = write(fd, buf, stor)) == -1) {
	//			//error
	//			return -1;
	//		}
	//		if (memchr(buf, '\n', sizeof(buf)) != NULL){
	//			break;
	//		}
	//	}
	//	if (lseek(fd, 0, SEEK_SET) == -1) {
	//		//error
	//		return -1;
	//	}
//
//		for (;;) {
//			stor = read(fd, buf, sizeof(buf));
//			if (stor == -1){
//				//error
//				return -1;
//			}
//			if (stor == 0) {
//				break;
//			}
//			if (send(newfd, buf, stor, 0) == -1) {
//				//error
//				return -1;
//			}
//		}
//		close(newfd);
//		syslog(LOG_INFO, "Closed connection from %s", s);
//	}

}

