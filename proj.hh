

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string.h>
#include <string>
#include <getopt.h>
#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>


using namespace std;

//global variables
char *server_no = NULL;
char *lfile =  NULL;
string sport;
int data_port = 0;
string searched_file;
char *file_path = NULL;
bool passive = false;
bool active = false;
bool rmv = false;
bool dwld = false;
bool upld = false;
ifstream log_file;

struct login_info
{
	string uname;
	string psswd;
} login;

struct dataSrvInfo
{
	char addr[15];
	int port;
} psvData;

void exitFunc(int number, string output);
void printHelp();
int optParser(int argc, char *argv[]);
void getLogInf();
int srvCommConnect();
void pasvDataConnect(struct hostent *name, int socfd);
int recvMsg(int sockfd, void *buf);