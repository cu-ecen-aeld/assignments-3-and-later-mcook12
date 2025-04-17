// Application to create new file with given filename and path and content
//Author: Michael Cook

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>

int main( int argc, char* argv[]){
	openlog (NULL, 0, LOG_USER);
	if (argv[2] == NULL){
		//error
		syslog(LOG_ERR, "Error: String not specified");
		return 1;
	}
	syslog (LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);
	const char *buf = strcat(argv[2], "\n");
	int fd;
	ssize_t nr;

	fd = open (argv[1], O_RDWR | O_CREAT | O_TRUNC, 0777);
	if (fd == -1){
		//error
		syslog(LOG_ERR, "Error opening file %s", argv[1]);
		return 1;
	}
	nr = write (fd, buf, strlen (buf));
	if (nr == -1){
		//error
		syslog(LOG_ERR, "Error writing to file %s", argv[1]);
		return 1;
	}
	return 0;
}

