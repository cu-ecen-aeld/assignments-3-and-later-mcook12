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

#define BUF_SIZE 1000
#define DATA_PATH "/var/tmp/aesdsocketdata"

int sockfd;
int newfd;
int fd;

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

int main(int argc, char *argv[]){
	char s[INET6_ADDRSTRLEN];
	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	ssize_t stor;
	char buf[BUF_SIZE];
	bool daemonize = false;
	int opt;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;

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
	
	if((fd = open(DATA_PATH, O_CREAT | O_RDWR, 0644)) == -1) {
		//error
                return -1;
	}
	
	signal(SIGINT, terminate);
	signal(SIGTERM, terminate);

	for(;;) {
		if ((newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) == -1) {
			//error
			return -1;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		syslog (LOG_INFO, "Accepted Connection from %s", s);

		if (lseek(fd, 0, SEEK_END) == -1) {
			//error
			return -1;
		}
		for (;;) {
			if((stor = recv(newfd, buf, sizeof(buf), 0)) == -1) {
				//error
				return -1;
			}
			if((stor = write(fd, buf, stor)) == -1) {
				//error
				return -1;
			}
			if (memchr(buf, '\n', sizeof(buf)) != NULL){
				break;
			}
		}
		if (lseek(fd, 0, SEEK_SET) == -1) {
			//error
			return -1;
		}

		for (;;) {
			stor = read(fd, buf, sizeof(buf));
			if (stor == -1){
				//error
				return -1;
			}
			if (stor == 0) {
				break;
			}
			if (send(newfd, buf, stor, 0) == -1) {
				//error
				return -1;
			}
		}
		close(newfd);
		syslog(LOG_INFO, "Closed connection from %s", s);
	}

}

