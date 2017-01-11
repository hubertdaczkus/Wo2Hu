//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
//#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cerrno>
#include <string.h>
#include <vector>
//#include <signal.h>
//#include <sys/wait.h>
//#include <pthread.h>
//#include <sys/select.h>

#define BUFROZ 256
#define ERROR(e) { perror(e); exit(EXIT_FAILURE); }
#define SERVER_PORT 1234
#define QUEUE_SIZE 5

using namespace std;

class User {
    public:
    string status;
    string token;
    string nickname;
    string pass;
    User(string, string, string);
};

User::User(string name, string pw, string stat) {
    status = stat;
    token = "";
    nickname = name;
    pass = pw;
}

class Connection {
    public:
    User* user;
    string message;
    string splittedMessage[100];
    int splitMessage(string);
    void setup(string, User);
};

int Connection::splitMessage(string str) {
    string tmp;
    int count = 0;
    for (int i = 0; i < str.length(); i++) {
        string curr;
        curr = str[i];
        if (strcmp(curr.c_str(), ";") != 0) {
            tmp += str[i];
        }
        else {
            splittedMessage[count] = tmp;
            tmp = "";
            count++;
        }
    }
    return count;
}

void Connection::setup(string msg, User usr) {
    message = msg;
    user = &usr;
}

string getFile(const char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (fp)
  {
    string contents;
    fseek(fp, 0, SEEK_END);
    contents.resize(ftell(fp));
    rewind(fp);
    fread(&contents[0], 1, contents.size(), fp);
    fclose(fp);
    return(contents);
  }
  throw(errno);
}

string sendMessage(User usr) {
    string str;
    string path = "MESSAGES/" + usr.nickname;
    str = getFile(path.c_str());
    cout << str << endl;
    return str;
}

string sendUsers(vector<User>& list) {
    string str;
    stringstream ss;
    ss << list.size();
    str += "00;";
    str += (ss.str() + ";");
    for (int i = 0; i < list.size(); i++){
        str += list[i].nickname + ";" + list[i].status + ";";
    }
    return str;
}

int main(int argc, char** argv) {
    char buf[BUFROZ];
    vector<User> users;
    User usr = User("admin", "1234", "0");

    users.push_back(User("admin", "1234", "0"));
    users.push_back(User("admin2", "1234", "0"));
    users.push_back(User("admin3", "1234", "0"));
    users.push_back(User("admin4", "1234", "0"));
    Connection conn;
    for (int i = 0; i < conn.splitMessage(sendUsers(users)); i++){
       cout << conn.splittedMessage[i] << endl;
    }
    cout << sendUsers(users) << endl;
    sendMessage(usr);

    socklen_t slt;
    int x, sfd, cfd, fdmax, fda, rc, i, j, on = 1;
    struct sockaddr_in saddr, caddr;
    static struct timeval timeout;
    fd_set mask, rmask, wmask;

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(SERVER_PORT);

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) ERROR("socket()");
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) ERROR("setsockopt()");
    if (bind(sfd, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) ERROR("bind()");
    if (listen(sfd, QUEUE_SIZE) < 0) ERROR("listen()");

    FD_ZERO(&mask);
    FD_ZERO(&rmask);
    FD_ZERO(&wmask);
    fdmax = sfd;

    while(1) {
        FD_SET(sfd, &rmask);
        wmask = mask;
        timeout.tv_sec = 5 * 60;
        timeout.tv_usec = 0;
        if ((rc = select(fdmax+1, &rmask, &wmask, (fd_set*)0, &timeout)) < 0) {
            ERROR("select()");
        }
        if (rc == 0) {
            cout << "timed out" << endl;
            continue;
        }
        fda = rc;
        if (FD_ISSET(sfd, &rmask)) {
            fda -= 1;
            slt = sizeof(caddr);
            if ((cfd = accept(sfd, (struct sockaddr*)&caddr, &slt)) < 0) {
                ERROR("accept()");
            }
            printf("new connection: %s\n", inet_ntoa((struct in_addr)caddr.sin_addr));
            FD_SET(cfd, &mask);
            if (cfd > fdmax) fdmax = cfd;
        }
        for (i = sfd+1; i <= fdmax && fda > 0; i++) {
            if (FD_ISSET(i, &wmask)) {
                fda -= 1;
                j = read(cfd, buf, BUFROZ);
                x = 0;
                for (j = 0; j < 4; j++) {
                    //if (strcmp(buf, lista[j].c_str()) == 0) {
                        //write(cfd, lista[j+1].c_str(), lista[j+1].length());
                        x = 1;			
                    //}
                }	
                if (x == 0) {	
                    write(cfd, "Brak indexu w bazie", 20);
                }
                close(i);
                FD_CLR(i, &mask);
                if (i == fdmax) {
                    while(fdmax > sfd && !FD_ISSET(fdmax, &mask)) {
                        fdmax -= 1;
                    }
                }
            }
        }
    }
    return EXIT_SUCCESS;
}