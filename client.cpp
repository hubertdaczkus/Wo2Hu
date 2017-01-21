//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <string>
#include <cerrno>
#include <string.h>
#include <vector>
//#include <signal.h>
//#include <sys/wait.h>
//#include <pthread.h>
//#include <sys/select.h>

#define BUFROZ 200000

using namespace std;

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Podaj parametry [hostname] [port] [\"msg\"]\n");
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
	string message = argv[3];
	int messageSize = message.length();
	int messageType = 3;
	
	write(fd, (void *)&messageSize, sizeof(messageSize));
	cout << "CLIENT: messageSize write: " << messageSize << endl;
	write(fd, (void *)&messageType, sizeof(messageType));
	cout << "CLIENT: messageType write: " << (int)messageType << endl;
	write(fd, message.c_str(), messageSize);
	cout << "CLIENT: message write: " << message << endl;
	
	read(fd, (void*)&messageSize, sizeof(messageSize));
	cout << "CLIENT: messageSize read: " << messageSize << endl;
    read(fd, (void*)&messageType, sizeof(messageType));
	cout << "CLIENT: messageType read: " << (int)messageType << endl;
	read(fd, buf, messageSize);
	cout << "CLIENT: message read: " << buf << endl;
 	
	close(fd);
	return 0;
}