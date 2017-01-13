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

#define BUFROZ 2
#define ERROR(e) { perror(e); exit(EXIT_FAILURE); }
#define SERVER_PORT 1234
#define QUEUE_SIZE 5

using namespace std;

class User {
    public:
    string status;
    string lastSeen;
    string token;
    string nickname;
    string password;
    User(string, string, string);
};

User::User(string name, string pw, string stat) {
    status = stat;
    token = "";
    nickname = name;
    password = pw;
    token = "";
}

class Connection {
    public:
    User* user;
    string message;
    string splittedMessage[100];
    int splitMessage(string);
};

int Connection::splitMessage(string str) {
    message = str;
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

string getFile(const char *filename)
{
    ifstream in(filename, ios::in | ios::binary);
    if (in)
    {
        string contents;
        in.seekg(0, ios::end);
        contents.resize(in.tellg());
        in.seekg(0, ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

void sendToFile(string input, string user) {
    string path = "MESSAGES/" + user;
    std::ofstream out(path.c_str());
    out << input;
    out.close();
}

string generateToken(int length)
{
    string token;
    string charmap = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < length; i++){
        token += charmap[rand() % charmap.length()];
    }
    return token;
}

string login(vector<User>& list,string uLogin, string uPass){
    string token = "";
    for (int i = 0; i < list.size(); i++) {
        if (list[i].nickname.compare(uLogin) == 0) {
            if (list[i].password.compare(uPass) == 0) {
                token = generateToken(10);
                list[i].token = token;
            } 
        }
    }
    return token;
}

const string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

string sendMessage(User usr) {
    string str, msg;
    stringstream ss;
    string path = "MESSAGES/" + usr.nickname;
    msg = getFile(path.c_str());
    ss << msg.size();
    str += "01;" + ss.str() + ";" + msg;
    cout << str << endl;
    return str;
}

string saveMessage(string msg, string usr) {
    string str;
    str += "<message from=\"" + usr + "\" date=\"" + currentDateTime() + "\">" + msg + "</message>";
    sendToFile(str, usr);
    return str.length();
}

string sendUsers(vector<User>& list) {
    string str;
    stringstream ss;
    ss << list.size();
    str += "01;";
    str += (ss.str() + ";");
    for (int i = 0; i < list.size(); i++){
        str += list[i].nickname + ";" + list[i].status + ";";
    }
    return str;
}

User* findToken(vector<User>& list, string token){
    for (int i = 0; i < list.size(); i++) {
        if (list[i].token == token) return &list[i];
    }
}

bool findUser(vector<User>& list, string nickname){
    for (int i = 0; i < list.size(); i++) {
        if (list[i].nickname == nickname) return true;
    }
    return false;
}

int main(int argc, char** argv) {
    srand (time(NULL));
    int messageSize;
    char buf[BUFROZ];
    string recMsg;
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
    cout << generateToken(10) << endl;
    cout << currentDateTime() << endl;

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
            cout << "new connection: " << inet_ntoa((struct in_addr)caddr.sin_addr) << endl;
            FD_SET(cfd, &mask);
            if (cfd > fdmax) fdmax = cfd;
        }
        for (i = sfd+1; i <= fdmax && fda > 0; i++) {
            if (FD_ISSET(i, &wmask)) {
                fda -= 1;
                read(cfd, buf, 4);
                messageSize = buf;
                while (j = read(cfd, buf, BUFROZ) > 0) {
                    recMsg += buf;
                    cout << recMsg << endl;
                }
                cout << recMsg << endl;
                conn.splitMessage(recMsg);
                if (conn.splittedMessage[0].compare("00") == 0) { //LOGIN 00;login;hasło;
                    string sendMsg = login(users, conn.splittedMessage[1],conn.splittedMessage[1]);
                    write(cfd, sendMsg.c_str(), sendMsg.length());
                }
                else if (conn.splittedMessage[0].compare("01") == 0) { //SEND USERS 01;token; 
                    conn.user = findToken(users, conn.splittedMessage[1]);
                    if (conn.user->token.compare(conn.splittedMessage[1]) == 0 ) {
                        string sendMsg = sendMessage(*conn.user);
                        write(cfd, sendMsg.c_str(), sendMsg.length());
                    }
                    
                }
                else if (conn.splittedMessage[0].compare("02") == 0) { //SEND MESSAGE 02;token; 
                    conn.user = findToken(users, conn.splittedMessage[1]);
                    if (conn.user->token.compare(conn.splittedMessage[1]) == 0 ) {
                        string sendMsg = sendUsers(users);
                        write(cfd, sendMsg.c_str(), sendMsg.length());
                    }
                }
                else if (conn.splittedMessage[0].compare("03") == 0) { //GET MESSAGE 03;token;messageSize;message; 
                    conn.user = findToken(users, conn.splittedMessage[1]);
                    if (conn.user->token.compare(conn.splittedMessage[1]) == 0 ) {
                        int prefixLen = conn.splittedMessage[0].length() + conn.splittedMessage[1].length() + conn.splittedMessage[2].length() + 3;
                        string saveStr = recMsg.substr(prefixLen, recMsg.length() - (prefixLen+1));
                        saveMessage(saveStr, conn.user->nickname);
                        //write(cfd, sendMsg.c_str(), sendMsg.length());
                    }
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