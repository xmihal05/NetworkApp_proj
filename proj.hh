/*
	project - FTP client
	Author: Klara Mihalikova <xmihal05>
*/

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string.h>
#include <string>
#include <getopt.h>
#include <fstream>
#include <fcntl.h>
#include <sstream>
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/sendfile.h>	//upload file to server
#include <sys/stat.h>	
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>

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

void exitFunc(int number, string output, int cSfd, int dSfd);
void printHelp();
int optParser(int argc, char *argv[]);
void getLogInf();
int recvMsg(int sockfd, void *buf);
void pasvDataConnect(struct hostent *name, int cSfd, string path);
void actDataConnect (int cSfd, string path);
int srvCommConnect();
//void printDIR(struct hostent *name, char mNLst[], char mPasv[], char setBinary[], string line, int cSfd);