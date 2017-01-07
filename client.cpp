#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFROZ 256

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Podaj parametry [hostname] [port]\n");
		return -1;
	}	
	int fd = socket(PF_INET, SOCK_STREAM, 0);
	//int fd = socket(PF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addr;
	struct hostent* ent = gethostbyname(argv[1]);
	memcpy(&addr.sin_addr.s_addr, ent -> h_addr, ent -> h_length);
	addr.sin_family = PF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	connect(fd, (struct sockaddr*) &addr, sizeof(addr));
	char buf[BUFROZ];
    char* message = "71764";
    write(fd, message, BUFROZ);
    int i = read(fd, buf, BUFROZ);
	buf[i] = '\0';
	printf("%s\n", buf);
 	close(fd);
	return 0;
}