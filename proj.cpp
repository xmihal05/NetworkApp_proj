/*
	project - FTP client
	Author:
*/

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string.h>
#include <string>
#include <getopt.h>
#include <fstream>

using namespace std;
//global variables
char *server_no = NULL;
char *lfile =  NULL;
char *sport = NULL;
int port = 0;
char *searched_file = NULL;
char *file_path = NULL;
bool fp = false;
bool passive = false;


void printHelp(){
	cout << "" << endl;
	cout << "Console FTP client for transfering files" << endl;
	cout << "" << endl;
	cout << "Run:" << endl;
	cout << "./fclient [-h/--help] -s 192.168.1.102 -c credentials.txt [-p]|[-a port_number] [-d|-u|-r filename] [-P path]" << endl;
	cout << "" << endl;
	cout << "-h/--help\tPrints out this help message" << endl;
	cout << "-s server\tIP address or domain name of server" << endl;
	cout << "-c filename\tFile containing user name and password" << endl;
	cout << "-a port_number\tActive mode, data connection is inicialized on this port" << endl;
	cout << "-p\t\tPassive mode" << endl;
	cout << "-u filename\tFile, which will be transfered to server" << endl;
	cout << "\t\tIn combination with -P will be the file saved on a specified directory" << endl;
	cout << "-d filename\tFile, which will be downloaded from server" << endl;
	cout << "\t\tIn combination with -P will be the file downloaded to a specified directory" << endl;
	cout << "-r filename\tFile, which will be deleted from server" << endl;
	cout << "\t\tIn combination with -P will be the file searched and deleted from a specified directory" << endl;
	cout << "-P path\t\tPath to a file, it is used only with parameters -d/-r/-u" << endl;
	cout << "" << endl;
	cout << "Required arguments are -s and -c." << endl;
	cout << "Arguments -a and -p are not combinable. Also arguments -u, -d and -r." << endl;
	cout << "" << endl;
	cout << "" << endl;
}

/*Function for exiting - prints out code and possibly error message*/
void exitFunc(int code){

	/* close login file before exit */	
	//if(log_file->is_open())
	//	log_file->close();

	switch(code){
		case 0:
			cout << "everything went smoothly\n";
			exit(0);
		case 1:
			cout << "Wrong use of parameters! Run ./fclien -h for help.\n";
			exit(1);
		case 2:
			cout << "Couldn't open login file\n";
			exit(1);
		case 3:
			cout << "Port number is not valid\n";
			exit(1);
		case 4:
			cout << "Use of uncombinable options or multiple use of any! Run ./fclien -h for help\n";
			exit(1);
		default:
			cout << "cant you even use your own function!? dumbass!! -_-\n";
			exit(1);
		/*case 0:
			cout << "";
			exit();*/
	}
}

int optParse(int argc, char *argv[]){

	//too many options used on input
	if(argc > 10)
		exitFunc(1);
	//required options werent used
	else if(argc < 5 && strcmp(argv[1],"-h") != 0)
		exitFunc(1);
	else{
		int opt, csf, cpa = 0;	//csf,cpa - counter for determination double use or wrong combination of options
		/*getopt learned from site - https://linux.die.net/man/3/getopt */
		while((opt = getopt(argc, argv, "hs:c:a:pu:d:r:P")) != -1){
			switch(opt){
				case 'h':
					if (argc != 2)
						exitFunc(1);
					printHelp();
					exitFunc(0);
				case 's':
					server_no = optarg;
					break;
				case 'c':
					lfile = optarg;
					break;
				case 'a':
					cpa++;
					sport = optarg;
					port = atoi(sport);
					if (port == 0 && strcmp(sport, "0") != 0)
						exitFunc(3);
					break;
				case 'p':
					cpa++;
					passive = true;
					break;
				case 'u':
					csf++;
					searched_file = optarg;
					break;
				case 'd':
					csf++;
					searched_file = optarg;
					break;
				case 'r':
					csf++;
					searched_file = optarg;
					break;
				case 'P':
					fp = true;
					file_path = optarg;
					break;
				default:
					exitFunc(1);
			}
		}
		
		if (csf > 1 || cpa > 1 || (csf == 0 && fp))
			exitFunc(4);
	}

	return 1;
}

/**********************************************************************************************/

int main(int argc, char *argv[]){

	optParse(argc,argv);
	exitFunc(0);
}