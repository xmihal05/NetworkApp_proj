/*
	project - FTP client
	Author:
*/

#include "proj.hh"

using namespace std;

/*Function for exiting - prints out code and possibly error message*/
void exitFunc(int code){

	/* close login file before exit */	
	if(log_file.is_open())
		log_file.close();

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
		case 5:
			cout << "Login file couldn't be opened or is not valid\n";
			exit(1);
		default:
			cout << "cant you even use your own function!? dumbass!! -_-\n";
			exit(1);
		/*case 0:
			cout << "";
			exit();*/
	}
}

/* Prints help message when -h option is used */
void printHelp(){
	cout << "\n";
	cout << "Console FTP client for transfering files\n";
	cout << "\n";
	cout << "Run:\n";
	cout << "./fclient [-h/--help] -s 192.168.1.102 -c credentials.txt [-p]|[-a port_number] [-d|-u|-r filename] [-P path]\n";
	cout << "\n";
	cout << "-h/--help\tPrints out this help message\n";
	cout << "-s server\tIP address or domain name of server\n";
	cout << "-c filename\tFile containing user name and password\n";
	cout << "-a port_number\tActive mode, data connection is inicialized on this port\n";
	cout << "-p\t\tPassive mode\n";
	cout << "-u filename\tFile, which will be transfered to server\n";
	cout << "\t\tIn combination with -P will be the file saved on a specified directory\n";
	cout << "-d filename\tFile, which will be downloaded from server\n";
	cout << "\t\tIn combination with -P will be the file downloaded to a specified directory\n";
	cout << "-r filename\tFile, which will be deleted from server\n";
	cout << "\t\tIn combination with -P will be the file searched and deleted from a specified directory\n";
	cout << "-P path\t\tPath to a file, it is used only with parameters -d/-r/-u\n";
	cout << "\n";
	cout << "Required arguments are -s and -c.\n";
	cout << "Arguments -a and -p are not combinable. Also arguments -u, -d and -r.\n";
	cout << "\n";
	cout << "\n";
}

/*function for parsing oprions and their arguments*/
int optParser(int argc, char *argv[]){

	//too many options used on input
	if(argc > 11)
		exitFunc(1);
	//required options werent used
	else if(argc < 5 && strcmp(argv[1],"-h") != 0)
		exitFunc(1);
	else{
		bool fp = false;
		bool is_number = false;
		int opt, csf, cpa,req, path = 0;	//csf,cpa - counter for determination double use or wrong combination of options
			// req counts required arguments; path - checks if path wasnt stated more than once
		/*getopt learned from site - https://linux.die.net/man/3/getopt */
		while((opt = getopt(argc, argv, "hs:c:a:pu:d:r:P")) != -1){
			switch(opt){
				case 'h':
					if (argc != 2)
						exitFunc(1);
					printHelp();
					exitFunc(0);
				case 's':
					req++;
					server_no = optarg; /*saves server address or domain name*/
					break;
				case 'c':
					req++;
					lfile = optarg; /*saves file used for autentization */
					break;
				case 'a':
					cpa++;
					sport = optarg;
					is_number = (sport.find_first_not_of( "0123456789" ) == string::npos); //check if string does contain NAN character
					if (is_number)
						port = atoi(sport.c_str());	//converts string to int
					else exitFunc(3); //strin isnt an integer, exits with error					
					break;
				case 'p':
					cpa++;
					passive = true; /*passive mode is used */
					break;
				case 'u':
					upld = true;
					csf++;
					searched_file = optarg; /*saves name of the file for upload*/
					break;
				case 'd':
					dwld = true;
					csf++;
					searched_file = optarg; /*saves name of the file for download*/
					break;
				case 'r':
					rmv = true;
					csf++;
					searched_file = optarg; /*saves name of the file for removal*/
					break;
				case 'P':
					path++;
					fp = true;
					file_path = optarg; /*saves path to a searched file*/
					break;
				default:
					exitFunc(1);
			}
		}

		/*if any parameter was used more than once, or uncombinable were combined - exit with error*/
		if (csf > 1 || cpa > 1 || (csf == 0 && fp) || (path > 1) || (server_no == NULL) || (lfile == NULL) || (req > 2))
			exitFunc(4);
	}

	return 1;
}
//function for login
void getLogInf(){
	string line, tmp1,tmp2;
	int i;

	while(getline(log_file, line)){
		i++;
		if (i > 2)	// login file has more than two expected rows
			exitFunc(5);
		tmp1 = line.substr(0,10);
		tmp2 = line.substr (10);
		if (tmp2.compare("") == 0)	//string is empty
			exitFunc(5);

		if (i==1){	//first line of login file holds username
			if (tmp1.compare("username: ") != 0) 	
				exitFunc(5);	//login file is not valid
			else login.uname = tmp2;
		}
		else{ //second line of login file holds password
			if (tmp1.compare("password: ") != 0)	
				exitFunc(5);	//login file is not valid
			else login.psswd = tmp2;
		}
	} 
}

/****************************************	MAIN 	****************************************/

int main(int argc, char *argv[]){

	optParser(argc,argv);


	//open input file containing autentization details
	log_file.open(lfile, ios::in);
	if (!log_file.is_open())	//throw error if file couldnt open
		exitFunc(5);

	getLogInf(); //get login info
	//structure login now holds login informations



	exitFunc(0);
}