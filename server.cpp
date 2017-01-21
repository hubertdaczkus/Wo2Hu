//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
//#include <netdb.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <string>
#include <cerrno>
#include <string.h>
#include <vector>
#include <dirent.h>
//#include <boost/filesystem.hpp>
//#include <signal.h>
//#include <sys/wait.h>
//#include <pthread.h>
//#include <sys/select.h>

#define BUFROZ 200000
#define ERROR(e) { perror(e); exit(EXIT_FAILURE); }
#define SERVER_PORT 1234
#define QUEUE_SIZE 5

using namespace std;

class User;
void printUsers(vector<User>&);
string getFile(const char);
void sendToFile(string, string, string);
string generateToken(int);
User* login(vector<User>&, string, string);
const string currentDateTime();
string prepareMessage(User, string);
int saveMessage(string, string, string);
string sendUsers(vector<User>&);
User* findToken(vector<User>&, string);
bool findUser(vector<User>&, string);

class User {
    public:
    string status;
    int lastSeen;
    string token;
    string nickname;
    string password;
    User(string, string, string);
};

User::User(string name, string pw, string stat) {
    status = stat;
    token = generateToken(2);
    nickname = name;
    password = pw;
}

class Connection {
    public:
    User* user;
    string message;
    string splittedMessage[100];
    int splitMessage(string);
};

int Connection::splitMessage(string str)
{
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

template <typename T>
string NumberToString ( T Number )
{
	stringstream ss;
	ss << Number;
	return ss.str();
}

void printUsers(vector<User>& list)
{
    for (int i = 0; i < list.size(); i++) {
        cout << list[i].nickname << " : " << list[i].token << " : " << list[i].status << endl;
    }
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

void sendToFile(string input, string fromUsr, string toUsr)
{
    string dir = "mkdir -p ./MESSAGES/" + toUsr;
    system(dir.c_str());
    dir = "mkdir -p ./MESSAGES/" + fromUsr;
    system(dir.c_str());
    string path = "./MESSAGES/" + toUsr + "/" + fromUsr;
    ofstream out;
    out.open(path.c_str(), ofstream::app | fstream::out);
    out << input;
    out.close();
    path = "./MESSAGES/" + fromUsr + "/" + toUsr;
    out.open(path.c_str(), ofstream::app | fstream::out);
    out << input;
    out.close();
}

void clearFile(string user) {
    string path = "./MESSAGES/" + user;
    ofstream ofs;
    ofs.open(path.c_str(), ofstream::out | ofstream::trunc);
    ofs.close();
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

User* login(vector<User>& list,string uLogin, string uPass)
{
    string token = "";
    for (int i = 0; i < list.size(); i++) {
        if (list[i].nickname.compare(uLogin) == 0) {
            if (list[i].password.compare(uPass) == 0) {
                token = generateToken(2);
                list[i].token = token;
                return &list[i];
            } 
        }
    }
    return 0;
}

const string currentDateTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%d-%m-%Y %X", &tstruct);
    return buf;
}

string prepareMessage(User usr, string fromUsr) {
    string msg;
    string path = "./MESSAGES/" + usr.nickname + "/" + fromUsr;
    msg = getFile(path.c_str());
    cout << msg << endl;
    return msg;
}

void deleteMessage(User usr) {
    stringstream ss;
    string path = "./MESSAGES/" + usr.nickname;
    remove(path.c_str());
}

int saveMessage(string msg, string fromUsr, string toUsr) {
    string str;
    str += "<msg from=\"" + fromUsr + "\" date=\"" + currentDateTime() + "\" size=\"" /*+ NumberToString(msg.length())*/ + "\">" + msg + "</msg>\n";
    sendToFile(str, fromUsr ,toUsr);
    return str.length();
}

string sendUsers(vector<User>& list, User usr) {
    string str;
    //stringstream ss;
    //ss << list.size();
    //str += (ss.str() + ";");
    for (int i = 0; i < list.size(); i++){
        if (list[i].nickname.compare(usr.nickname)) str += list[i].nickname + ";" + list[i].status + ";";
    }
    return str;
}

User* findToken(vector<User>& list, string token)
{
    for (int i = 0; i < list.size(); i++) {
        if (list[i].token == token) return &list[i];
    }
    return 0;
}

bool findUser(vector<User>& list, string nickname)
{
    for (int i = 0; i < list.size(); i++) {
        if (list[i].nickname == nickname) return true;
    }
    return false;
}

int updateStatus(vector<User>& list, int lastTime) {
    int actualTime = time(0);
    cout << "LastTime: " << lastTime << endl;
    cout << "ActualTime: " << actualTime << endl;
    if (actualTime - lastTime > 5) {
        for (int i = 0; i < list.size(); i++) {
            if (actualTime - list[i].lastSeen > 5) {
                list[i].status = "0";
                cout << "Wyzerowano status dla: " << list[i].nickname << endl;
            }
        }
        return actualTime;
    }
    return lastTime;
}

void listFile(){
        DIR *pDIR;
        struct dirent *entry;
        if( pDIR=opendir("./MESSAGES/") ){
                while(entry = readdir(pDIR)){
                        if( strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 )
                        cout << entry->d_name << "\n";
                }
                closedir(pDIR);
        }
}
void writeMessage(int desc, int msgSize, int msgType, string msg){
    write(desc, (void*)& msgSize, sizeof(msgSize));
    cout << "SERVER: messageSize write: " << msgSize << endl;
    write(desc, (void*)& msgType, sizeof(msgType));
    cout << "SERVER: messageType write: " << msgType << endl;
    write(desc, msg.c_str(), msgSize);
    cout << "SERVER: message write: " << msg << endl;
}

int main(int argc, char** argv) {
    listFile();
    srand (time(NULL));
    int lastTime = time(0);
    vector<User> users;
    User usr = User("admin", "1234", "0");
    users.push_back(User("admin", "1234", "0"));
    users.push_back(User("admin2", "1234", "0"));
    users.push_back(User("admin3", "1234", "0"));
    users.push_back(User("admin4", "1234", "0"));
    saveMessage("test", "admin", "admin2");
    saveMessage("test2", "admin", "admin2");
    saveMessage("test3", "admin", "admin2");
    saveMessage("test4", "admin", "admin2");
    saveMessage("test5", "admin", "admin2");
    saveMessage("test6", "admin", "admin2");
    saveMessage("test7", "admin", "admin2");
    saveMessage("test8", "admin", "admin2");
    
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
                lastTime = updateStatus(users, lastTime);
                int messageSize, messageType;
                char buf[BUFROZ];
                string recMsg, sendMsg;
                fda -= 1;
                read(cfd, (void*)&messageSize, sizeof(messageSize));
                cout << "SERVER: messageSize read: " << messageSize << endl;
                read(cfd, (void*)&messageType, sizeof(messageType));
                cout << "SERVER: messageType read: " << messageType << endl;
                read(cfd, buf, messageSize);
                buf[messageSize] = '\0';
                recMsg = buf;
                cout << "SERVER: message read: " << recMsg << endl;
                Connection conn;
                conn.splitMessage(recMsg);
                if ( messageType == 0) { //LOGIN login;hasło;
                    cout << "SERVER: Choose type 0" << endl;
                    conn.user = login(users, conn.splittedMessage[0], conn.splittedMessage[1]);
                    sendMsg = "";
                    if (conn.user) sendMsg = conn.user->token;
                    if (sendMsg.compare("") != 0) {
                        messageSize = sendMsg.length();
                        writeMessage(cfd, messageSize, messageType, sendMsg);
                        conn.user->status = "1";
                        conn.user->lastSeen = time(0);                        
                    }
                    else {
                        messageType = 5;
                        sendMsg = "Uzytkownik nierozpoznany";
                        messageSize = sendMsg.length();
                        writeMessage(cfd, messageSize, messageType, sendMsg);
                    }
                    printUsers(users);

                }
                else if (messageType == 1) { //SEND USERS token; 
                    cout << "SERVER: Choose type 1" << endl;
                    if (findToken(users, recMsg)) {
                        conn.user = findToken(users, recMsg);
                        cout << "Auth Login: " << conn.user->nickname << " Token: " << conn.user->token << endl;
                        if (conn.user) {
                            sendMsg = sendUsers(users, *conn.user);
                            messageSize = sendMsg.length();
                            writeMessage(cfd, messageSize, messageType, sendMsg);
                            conn.user->status = "1";
                            conn.user->lastSeen = time(0);
                        }
                    }
                    else {
                        messageType = 5;
                        sendMsg = "Token nierozpoznany";
                        messageSize = sendMsg.length();
                        writeMessage(cfd, messageSize, messageType, sendMsg);;
                    }
                    printUsers(users);
                }
                
                
                else if (messageType == 2) { //SEND MESSAGE token;fromUsr; 
                    cout << "SERVER: Choose type 2" << endl;
                    if (findToken(users, conn.splittedMessage[0])) {
                        conn.user = findToken(users, conn.splittedMessage[0]);
                        cout << conn.splittedMessage[0] << "  ####   " << conn.splittedMessage[1] << endl;
                        cout << "Auth Login: " << conn.user->nickname << " Token: " << conn.user->token << endl;
                        if (conn.user) {
                            sendMsg = prepareMessage(*conn.user, conn.splittedMessage[1]);
                            messageSize = sendMsg.length();
                            writeMessage(cfd, messageSize, messageType, sendMsg);
                            conn.user->status = "1";
                            conn.user->lastSeen = time(0);
                            //clearFile(conn.user->nickname);
                        }
                    }
                    else {
                        messageType = 15;
                        cout << conn.splittedMessage[0] << "  ####   " << conn.splittedMessage[1] << endl;
                        sendMsg = "Token nierozpoznany";
                        messageSize = sendMsg.length();
                        writeMessage(cfd, messageSize, messageType, sendMsg);
                    }
                    printUsers(users);
                }
                else if (messageType == 3) { //GET MESSAGE token;toUsr;message 
                    cout << "SERVER: Choose type 3" << endl;
                    if (findToken(users, conn.splittedMessage[0])) {
                        conn.user = findToken(users, conn.splittedMessage[0]);
                        cout << "Auth Login: " << conn.user->nickname << " Token: " << conn.user->token << endl;
                        if (conn.user && findUser(users, conn.splittedMessage[1])) {
                            saveMessage(recMsg.substr(conn.splittedMessage[0].length() + conn.splittedMessage[1].length() + 2), conn.user->nickname, conn.splittedMessage[1]);
                            sendMsg = "true";
                            messageSize = sendMsg.length();
                            writeMessage(cfd, messageSize, messageType, sendMsg);
                            conn.user->status = "1";
                            conn.user->lastSeen = time(0);
                        }
                        else {
                            messageType = 5;
                            sendMsg = "Nie ma takiego uzytkownika";
                            messageSize = sendMsg.length();
                            writeMessage(cfd, messageSize, messageType, sendMsg);
                            conn.user->status = "1";
                            conn.user->lastSeen = time(0);
                        }
                    }
                    else {
                        messageType = 5;
                        sendMsg = "Token nierozpoznany";
                        messageSize = sendMsg.length();
                        writeMessage(cfd, messageSize, messageType, sendMsg);
                    }
                    printUsers(users);
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