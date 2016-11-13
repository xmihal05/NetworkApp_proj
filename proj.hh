

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
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


using namespace std;

//global variables
char *server_no = NULL;
char *lfile =  NULL;
string sport;
int port = 0;
char *searched_file = NULL;
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

void exitFunc(int code);
void printHelp();
int optParser(int argc, char *argv[]);
void getLogInf();
int srvConnect(struct sockaddr_in address);