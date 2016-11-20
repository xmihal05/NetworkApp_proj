/*
	project - FTP client
	Author:
*/

#include "proj.hh"

#define CM_PORT 21		//port for FTP command flow
#define B_SIZE 1024
#define DB_SIZE 4096		//size of buffer for messages received from server
#define M_SIZE 250		//size of messages send to server from client
#define M_SIZE_SHORT 20 //size of short messages send to server from client

#define MAX_PATHNAME_LEN 260
#define NUM_THREADS 5

using namespace std;

/*Function for exiting - prints out code and possibly error message*/
void exitFunc(int number, string output){

	/* close login file before exit */	
	if(log_file.is_open())
		log_file.close();

		cout << output;
		exit(number);
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
		exitFunc(1, "Wrong use of parameters! Run ./fclien -h for help.\n");
	//required options werent used
	else if(argc < 5 && strcmp(argv[1],"-h") != 0)
		exitFunc(1, "Wrong use of parameters! Run ./fclien -h for help.\n");
	else{
		bool is_number = false;
		int opt, csf = 0;	//csf,cpa - counter for determination double use or wrong combination of options
		int req = 0;
		int path = 0;
		int cpa = 0;
			// req counts required arguments; path - checks if path wasnt stated more than once
		/*getopt learned from site - https://linux.die.net/man/3/getopt */
		while((opt = getopt(argc, argv, "hs:c:a:pu:d:r:P:")) != -1){
			switch(opt){
				case 'h':
					if (argc != 2)
						exitFunc(1, "Wrong use of parameters! Run ./fclien -h for help.\n");
					printHelp();
					exitFunc(0, "everything went smoothly\n");
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
					active = true;
					sport = optarg;
					is_number = (sport.find_first_not_of( "0123456789" ) == string::npos); //check if string does contain NAN character
					if (is_number)
						data_port = atoi(sport.c_str());	//converts string to int
					else exitFunc(1, "Port number is not valid\n"); //strin isnt an integer, exits with error					
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
					file_path = optarg; /*saves path to a searched file*/
					break;
				default:
					exitFunc(1, "Wrong use of parameters! Run ./fclien -h for help.\n");
			}
		}

		/*if any parameter was used more than once, or uncombinable were combined - exit with error*/
		if (csf > 1 || cpa > 1 || (csf == 0 && (path > 0)) || (path > 1) || (server_no == NULL) || (lfile == NULL) || (req > 2))
			exitFunc(1, "Use of uncombinable options or multiple use of any! Run ./fclien -h for help\n");
	}

	return 1;
}
//function for login
void getLogInf(){
	string line, tmp1,tmp2;
	int i = 0;

	while(getline(log_file, line)){
		i++;
		if (i > 2)
			exitFunc(1, "Login file couldn't be opened or is not valid\n");	// login file has more than two expected rows
			
		tmp1 = line.substr(0,10);
		tmp2 = line.substr (10);

		if (tmp2.compare("") == 0) //string is empty
			exitFunc(1, "Login file couldn't be opened or is not valid\n");

		if (i==1){	//first line of login file holds username
			if (tmp1.compare("username: ") != 0)
				exitFunc(1, "Login file couldn't be opened or is not valid\n");	//login file is not valid
			else login.uname = tmp2;
		}
		else{ //second line of login file holds password
			if (tmp1.compare("password: ") != 0)
				exitFunc(1, "Login file couldn't be opened or is not valid\n");	//login file is not valid
			else login.psswd = tmp2;
		}
	} 
}

int recvMsg(int sockfd, void *buf){
	int code, tmp;
	string code_str;	

	//save message received from server into buffer
	if((tmp = recv(sockfd, buf, B_SIZE, 0)) < 0)
			exitFunc(1, "Error on receive\n");

	//cast buffer from void * to string
	string msg_str(static_cast<const char*>(buf));
	//get code from message and cast it to int
	code_str = msg_str.substr(0,3);
	code = atoi(code_str.c_str());

	//extracts port number from "Entering passive mode" message
	if(code == 227){
		string psv_str;
		int ip1,ip2,ip3,ip4,port1, port2;
		//saves ip address
		psv_str = msg_str.substr(26);
		//inspired by stackoverflow.com/questions/33245774/simple-ftp-client-c
		sscanf(psv_str.c_str(), "(%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2);	
		//calculate port number for passive data connection
		psvData.port = (port1*256) + port2;
		//store ip address for passive data connection
		sprintf(psvData.addr, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
	}

	return code;
}

/********************************************************************************************
*********************************************************************************************
*********************************************************************************************/

void pasvDataConnect(struct hostent *name, int cSfd, string path){
	int dSfd, msg_code, fd, filesize;
	char buf[B_SIZE] = {0};
	char dataBuf[DB_SIZE] = {0};
	struct sockaddr_in dataAddr;
	struct stat obj;

	char setBinary[M_SIZE] = {0};
	char file[B_SIZE] = {0};
	char mStor[M_SIZE] = {0};
	char mRet[M_SIZE] = {0};
	char mNLst[M_SIZE_SHORT] = {0};
	char mPasv[M_SIZE_SHORT] = {0};

	strncpy(mPasv, "PASV\r\n", sizeof(mPasv));	
	strncpy(setBinary, "TYPE I\r\n", sizeof(setBinary));	//set flag to binary

	if(upld == true || dwld == true){
		if(file_path != NULL){
			if(upld){
				string sfile;
				sfile = "STOR "+path+searched_file+"\r\n"; //create stor command with path
				strncpy(file, searched_file.c_str(), sizeof(file));
				strncpy(mStor, sfile.c_str(), sizeof(mStor));
			}
			else{	
				string rfile;
				rfile = "RETR "+searched_file+"\r\n";

				string tmp_str;
				tmp_str = "~"+path+searched_file;
				strncpy(file, tmp_str.c_str(), sizeof(file));
				strncpy(mRet, rfile.c_str(), sizeof(mRet));
			}
		}		
		else{
			if(upld){
				string sfile;
				sfile = "STOR "+searched_file+"\r\n"; //store command without path
				strncpy(mStor, sfile.c_str(), sizeof(mStor));
			}
			else{
				string rfile;
				rfile = "RETR "+searched_file+"\r\n";
				strncpy(mRet, rfile.c_str(), sizeof(mRet));
			}
			strncpy(file, searched_file.c_str(), sizeof(file));			
		}
	}
	else
		strncpy(mNLst, "NLST\r\n", sizeof(mNLst));

	//create data socket
	if((dSfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exitFunc(1, "Data socket create error\n");
	bzero((char *) &dataAddr, sizeof(dataAddr));	//memory init - sets all values to zero
	dataAddr.sin_family = AF_INET;
	dataAddr.sin_port = htons(psvData.port);
	bcopy((char *)name->h_addr, (char *)&dataAddr.sin_addr.s_addr, name->h_length);

	//connect to server on data channel
	if(connect(dSfd, (struct sockaddr *)&dataAddr, sizeof(dataAddr)) < 0){
		cout << "connect: " << strerror(1) << "\n";
		exitFunc(1, "Failed to connect to a server\n");	//if connection cant be established - exit with error
	}

	//Turn the binary flag on
	if(send(cSfd, setBinary, strlen(setBinary), 0) < 0){		
			cout << "Set binary: " << strerror(1) << "\n";
			exitFunc(1, "Setting binary flag on failed\n");
	}
	msg_code = recvMsg(cSfd, buf);
	if(msg_code != 200)
		exitFunc(1, "Could not set binary flag on\n");

	/****************************************************************************************/
	//upload file to server
	if(upld){
		//INSPIRED BY: http://armi.in/wiki/FTP_Server_and_Client_Implementation_in_C/C%2B%2B
		//check if file for upload exists and is valid file
		fd = open(file, O_RDONLY);	//SKUSIT PRIDAT NEJAKU PERMISSION!!!
		if(fd < 0){
			exitFunc(1, "Searched file is not valid file or does not exists\n");
		}

		//send STOR command to communication channel
		if(send(cSfd, mStor, strlen(mStor),0) < 0){
			cout << "Storee send on connection channel: " << strerror(1) << "\n";
			close(fd);
			exitFunc(1, "Server error on sending STOR on CCH\n");
		}
		//get size of a file
		stat(file, &obj);	
		filesize = obj.st_size;
		//send file via data channel
		if(sendfile(dSfd, fd, NULL, filesize) < 0){			
			cout << "Sending file via data channel: " << strerror(1) << "\n";
			close(fd);
			exitFunc(1, "Error on sending file via data channel\n");
		}
		close(fd);

		//receive return code from server
		msg_code = recvMsg(cSfd, buf);
		if(msg_code != 150){
			exitFunc(1, "File transfer with errors - did not receive 150\n");
		}
		close(dSfd);	//close socket for data channel

		msg_code = recvMsg(cSfd, buf);	//wait for the transfer succesfull message
		if(msg_code != 226)
			exitFunc(1, "File transfered with errors - did not receive 226\n");
	}

	/****************************************************************************************/
	//download file from server
	else if(dwld){
		//open file for writting
		if((fd = open(file, O_WRONLY | O_CREAT, 0700)) < 0){	//open file for write only
			cout << "Opening file: " << strerror(1) << "\n";
			exitFunc(1, "Could not open file for download\n");
		}
		//send RETR command to communication channel
		if(send(cSfd, mRet, strlen(mRet),0) < 0){
			cout << "Retr send on connection channel: " << strerror(1) << "\n";
			exitFunc(1, "Server error on sending RETR on CCH\n");
		}
		//download file until everithing is received, and write it into open file
		while(1){
			if((filesize = recv(dSfd, dataBuf, DB_SIZE, 0))< 0){
				cout << "Getting download file size: " << strerror(1) << "\n";
				exitFunc(1, "Specified file doesn't exist in stated directory\n");
			}
			write(fd, dataBuf, filesize);	//write to a file
			//if there is nothing else to download exit loop, else continue
			if(filesize != DB_SIZE)
				break;
		}

		close(fd);	//close file
		close(dSfd);	//close data channel

		msg_code = recvMsg(cSfd, buf);	//wait for the transfer succesfull message
		if(msg_code != 150)
			exitFunc(1, "File transfered with errors - did not receive 150\n");

		msg_code = recvMsg(cSfd, buf);	//wait for the transfer succesfull message
		if(msg_code != 226)
			exitFunc(1, "File transfered with errors - did not receive 226\n");
		
	}

	/****************************************************************************************/
	//print out all the directories and subdirectories
	else if((upld == false)&&(dwld == false)&&(rmv == false)){
		if((fd = open("tmp", O_WRONLY | O_CREAT, 0644)) < 0){	//open file for write only
			cout << "Opening file: " << strerror(1) << "\n";
			exitFunc(1, "Could not open file for download\n");
		}
		//send NLST command
		if(send(cSfd, mNLst, strlen(mNLst),0) < 0){
			cout << "NLST send on connection channel: " << strerror(1) << "\n";
			exitFunc(1, "Server error on sending NLST on CCH\n");
		}

		if((filesize = recv(dSfd, dataBuf, DB_SIZE, 0)) < 0){
			cout << "Downloading list of directories: " << strerror(1) << "\n";
			exitFunc(1, "Failed to get directories\n");
		}

		//copy received message into temporary file
		write(fd, dataBuf, sizeof(dataBuf));
		close(fd);

		msg_code = recvMsg(cSfd, buf);	//wait for the transfer succesfull message
		if(msg_code != 150)
			exitFunc(1, "File transfered with errors - did not receive 150\n");

		msg_code = recvMsg(cSfd, buf);	//wait for the transfer succesfull message
		if(msg_code != 226)
			exitFunc(1, "File transfered with errors - did not receive 226\n");

		read(fd, dataBuf, filesize);
		string msg_str(static_cast<const char*>(dataBuf));
		cout << msg_str << endl;

		string line;
		string msg;
		while(getline(log_file, line)){
			if(send(cSfd, mPasv, strlen(mPasv),0) < 0){
				cout << "Passive mode message send: " << strerror(1) << "\n";
				exitFunc(1, "PASV wasnt send correctly \n");
			}
			msg_code = recvMsg(cSfd, buf);
			if(msg_code != 227)
				exitFunc(1, "Could not enter passive mode\n");

			if((fd = open("tmp2", O_WRONLY | O_CREAT, 0644)) < 0){	//open file for write only
				cout << "Opening file: " << strerror(1) << "\n";
				exitFunc(1, "Could not open file for download\n");
			}
			//poslat pasv, poslat nlst, ulozit do suboru a vypisat do terminalu
			msg = "NLST "+line;
			strncpy(mNLst, msg.c_str(), sizeof(mNLst));

			if(send(cSfd, mNLst, strlen(mNLst),0) < 0){
				cout << "NLST send on connection channel: " << strerror(1) << "\n";
				exitFunc(1, "Server error on sending NLST on CCH\n");
			}

			if((filesize = recv(dSfd, dataBuf, DB_SIZE, 0)) < 0){
				cout << "Downloading list of directories: " << strerror(1) << "\n";
				exitFunc(1, "Failed to get directories\n");
			}

			close(fd);
			read(fd, dataBuf, filesize);
			string msg_str(static_cast<const char*>(dataBuf));
			cout << msg_str << endl;

			remove("tmp2");
		}

		remove("tmp");
	}
	/****************************************************************************************/
}

/********************************************************************************************
*********************************************************************************************
*********************************************************************************************/
void *actMessage(void *threadid){
	
}
/********************************************************************************************
*********************************************************************************************
*********************************************************************************************/
void actDataConnect (int cSfd, string path){
	const char *ipaddr, *ifname = "eth0";	
	char mPort[M_SIZE] = {0};
	char setBinary[M_SIZE] = {0};
	char ports[20], buf[B_SIZE] = {0};
	struct ifaddrs *ifaddr, *ifa;
	int p1,p2, bSfd, lSfd, msg_code,fd, filesize;
	struct sockaddr_in my_addr, peer_addr;
	socklen_t pa_size;//peer address size
	string rfile, sfile;
	struct stat obj;

	strncpy(setBinary, "TYPE I\r\n", sizeof(setBinary));	//message for setting flag to binary

	if((strcmp(server_no, "localhost") == 0) || (strcmp(server_no,"127.0.0.1") == 0))
			ipaddr = "127.0.0.1";
	else{
		//inspired by https://gist.github.com/qxj/5618237
		if(getifaddrs(&ifaddr) < 0)
			exitFunc(1, "Error on get ifaddrs\n");

		for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
			if((ifa->ifa_addr != NULL) && (strcmp(ifa->ifa_name, ifname) == 0) && (ifa->ifa_addr->sa_family == AF_INET)){
				ipaddr = inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr);
				break;
			}
		}
		//free allocated memory
		freeifaddrs(ifaddr);
	}

	//create message for PORT command for server
	string addrString(ipaddr);
	//replace all dots with commas
	replace(addrString.begin(), addrString.end(), '.', ',');
	//divide port number
	p1 = data_port/256;	
	p2 = data_port % 256;
	//create PORT x,x,x,x,x,x message
	sprintf(ports, ",%d,%d\r\n", p1, p2);
	addrString = "PORT "+addrString+ports;
	cout << addrString << endl;
	strncpy(mPort, addrString.c_str(), sizeof(mPort));

	//create socket
	if((bSfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exitFunc(1,"Failed to create socket to bind\n");

	//allocate memory
	memset(&my_addr, 0, sizeof(struct sockaddr_in));
	//define options
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(data_port);
	my_addr.sin_addr.s_addr = inet_addr(ipaddr);

	//bind on local port for listening to server
	if(bind(bSfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) < 0){
		close(bSfd);
		exitFunc(1, "Failed to bind to a local port\n");
	}

	//listen
	if(listen(bSfd, 50) < 0)
		exitFunc(1, "Error on listening\n");

	//Turn the binary flag on
	if(send(cSfd, setBinary, strlen(setBinary), 0) < 0){		
			cout << "Set binary: " << strerror(1) << "\n";
			exitFunc(1, "Setting binary flag on failed\n");
	}
	msg_code = recvMsg(cSfd, buf);
	if(msg_code != 200)
		exitFunc(1, "Could not set binary flag on\n");

	//send PORT message to server via control channel
	if(send(cSfd, mPort, strlen(mPort),0) < 0){
			cout << "PORT send on connection channel: " << strerror(1) << "\n";
			exitFunc(1, "Server error on sending PORT on CCH\n");
	}
	msg_code = recvMsg(cSfd, buf);
	if(msg_code != 200)
		exitFunc(1, "PORT wasn't received correctly\n");

	/****************************************************************************************/
	//create and initialize commands
	char file[B_SIZE] = {0};
	char mStor[M_SIZE] = {0};
	char mRet[M_SIZE] = {0};
	char mNLst[M_SIZE_SHORT] = {0};

	if(upld == true || dwld == true){
		if(file_path != NULL){
			if(upld){
				string sfile;
				sfile = "STOR "+path+searched_file+"\r\n"; //create stor command with path
				strncpy(file, searched_file.c_str(), sizeof(file));
				strncpy(mStor, sfile.c_str(), sizeof(mStor));
			}
			else{	
				string rfile;
				rfile = "RETR "+searched_file+"\r\n";

				string tmp_str;
				tmp_str = path+searched_file;
				strncpy(file, tmp_str.c_str(), sizeof(file));
				strncpy(mRet, rfile.c_str(), sizeof(mRet));
			}
		}		
		else{
			if(upld){
				string sfile;
				sfile = "STOR "+searched_file+"\r\n"; //store command without path
				strncpy(mStor, sfile.c_str(), sizeof(mStor));
			}
			else{
				string rfile;
				rfile = "RETR "+searched_file+"\r\n";
				strncpy(mRet, rfile.c_str(), sizeof(mRet));
			}
			strncpy(file, searched_file.c_str(), sizeof(file));			
		}
	}
	else
		strncpy(mNLst, "NLST\r\n", sizeof(mNLst));

	/****************************************************************************************/
	//send command to server
	if(upld){
		//check if file for upload exists and is valid file
		fd = open(file, O_RDONLY);	//SKUSIT PRIDAT NEJAKU PERMISSION!!!
		if(fd < 0){
			exitFunc(1, "Searched file is not valid file or does not exists\n");
		}

		//send STOR command to communication channel
		if(send(cSfd, mStor, strlen(mStor),0) < 0){
			cout << "Storee send on connection channel: " << strerror(1) << "\n";
			close(bSfd);
			close(fd);
			exitFunc(1, "Server error on sending STOR on CCH\n");
		}
		//get size of a file
		stat(file, &obj);	
		filesize = obj.st_size;
		//receive return code from server
		msg_code = recvMsg(cSfd, buf);
		if(msg_code != 150){
			close(bSfd);
			exitFunc(1, "File transfer with errors - did not receive 150\n");
		}
		//send file via data channel
		if(sendfile(bSfd, fd, NULL, filesize) < 0){			
			cout << "Sending file via data channel: " << strerror(1) << "\n";
			close(bSfd);
			close(fd);
			exitFunc(1, "Error on sending file via data channel\n");
		}
		cout << "teraz som tu\n";

		//accept info from server
		pa_size = sizeof(struct sockaddr_in);
		if((lSfd = accept(bSfd, (struct sockaddr *) &peer_addr, &pa_size))<0){
			close(bSfd);
			close(fd);
			exitFunc(1, "Error on accept\n");
		}
		cout << "som tu\n";

		msg_code = recvMsg(cSfd, buf);	//wait for the transfer succesfull message
		if(msg_code != 226){
			close(bSfd);
			close(fd);
			exitFunc(1, "File transfered with errors - did not receive 226\n");
		}

		close(fd);
	}
	/****************************************************************************************/
	else if(dwld){
		cout << "DOwnload of file in active mode\n";	
	}
	/****************************************************************************************/
	else if((upld == false)&&(dwld == false)&&(rmv == false)){
		cout << "Print out directories\n";
	}
	/****************************************************************************************/

	close(bSfd);

}		

/********************************************************************************************
*********************************************************************************************
*********************************************************************************************/
int srvCommConnect(){
	struct sockaddr_in address;
	struct hostent *name;
	int sfd, msg_code;
	string dfile,fpath;
	char buffer[B_SIZE] = {0};

	//initialize arrays
	char mDel[M_SIZE] = {0};
	char mUser[M_SIZE] = {0};
	char mPass[M_SIZE] = {0};
	char mPasv[M_SIZE_SHORT] = {0};
	char mQuit[M_SIZE_SHORT] = {0};

	if(file_path != NULL){
		string tmp;

		string path(file_path);
		tmp = path.substr(0,1);
		//check if path starts with '/', if not add it to beginning of string
		if (tmp.compare("/") != 0){
			path = "/"+path;
		}
		tmp = path.substr(path.size() - 1);
		if (tmp.compare("/") != 0){
			path = path+"/";
		}

		//add path to the searched file
		dfile = "DELE "+path+searched_file+"\r\n";

		fpath = path;
	}

	else{
		//file should be inside foor directory
		dfile = "DELE "+searched_file+"\r\n";
	}

	string usrstr = "USER "+login.uname+"\r\n";
	string pswstr = "PASS "+login.psswd+"\r\n";
	
	//fill arrays with proper info
	strncpy(mDel, dfile.c_str(), sizeof(mDel));
	strncpy(mUser, usrstr.c_str(), sizeof(mUser));
	strncpy(mPass, pswstr.c_str(), sizeof(mPass));
	strncpy(mPasv, "PASV\r\n", sizeof(mPasv));	
	strncpy(mQuit, "QUIT\r\n", sizeof(mQuit));

	sfd = socket(AF_INET, SOCK_STREAM, 0);	
	if(sfd < 0)		//could not open socket - exit with error
		exitFunc(1, "Socket create error\n");

	bzero((char *) &address, sizeof(address));	//memory init - sets all values to zero

	address.sin_family = AF_INET;
	address.sin_port = htons(CM_PORT);	//connect to port 21 for command flow
	
	if((name = gethostbyname(server_no)) == NULL)	//get ip address to connect to a server
		exitFunc(1, "gethostbyname error\n");	//exit with errorS
	address.sin_addr = *((struct in_addr *)name->h_addr); 
	//}

	//connect to server
	if(connect(sfd, (struct sockaddr *)&address, sizeof(address)) < 0){
		cout << "connect: " << strerror(1) << "\n";
		exitFunc(1, "Failed to connect to a server\n");	//if connection cant be established - exit with error
	}
	msg_code = recvMsg(sfd, buffer);
	if((msg_code != 120) && (msg_code != 220))
		exitFunc(1, "Connection wasn't established\n");

	//send username &  password for loging to a server
	if(send(sfd, mUser, strlen(mUser),0) < 0){
		cout << "username send: " << strerror(1) << "\n";
		exitFunc(1, "Server error on sending USER\n");
	}
	msg_code = recvMsg(sfd, buffer);
	if((msg_code != 230) && (msg_code != 331))
		exitFunc(1, "Wrong user name or password\n");
	//send password only if it is needed
	if(msg_code == 331){
		if(send(sfd, mPass, strlen(mPass),0) < 0){
			cout << "password send: " << strerror(1) << "\n";
			exitFunc(1, "Server error on sending PASS\n");
		}
		msg_code = recvMsg(sfd, buffer);
		if(msg_code != 230)
			exitFunc(1, "Wrong user name or password\n");
	}
	

	//DELE doesnt need data channel open - so if wanted, proceed now
	if(rmv){
		if(send(sfd, mDel, strlen(mDel),0) < 0){
			cout << "Command DELE send: " << strerror(1) << "\n";
			exitFunc(1, "Server error on sending DELE\n");
		}
		msg_code = recvMsg(sfd, buffer);
		if(msg_code != 250)
			exitFunc(1, "File does not exists, or you don't have rights for deletion\n");
		else{
			if(send(sfd, mQuit, strlen(mQuit),0) < 0){
				cout << "Logging out: " << strerror(1) << "\n";
				exitFunc(1, "QUIT wasnt send correctly \n");
			}
			msg_code = recvMsg(sfd, buffer);
			if(msg_code != 221)
				exitFunc(1, "Failed to close connection \n");
			//close socket
			close(sfd);
			exitFunc(0, "Deletion and log out successful\n");
		}
	}

	//if active mode wasnt chosen work in passive
	if (!active){
		if(send(sfd, mPasv, strlen(mPasv),0) < 0){
		cout << "Passive mode message send: " << strerror(1) << "\n";
		exitFunc(1, "PASV wasnt send correctly \n");
		}
		msg_code = recvMsg(sfd, buffer);
		if(msg_code != 227)
			exitFunc(1, "Could not enter passive mode\n");
		//establish data connection in passive mode
		pasvDataConnect(name, sfd, fpath);
	}
	else actDataConnect(sfd, fpath);

	if(send(sfd, mQuit, strlen(mQuit),0) < 0){
		cout << "Logging out: " << strerror(1) << "\n";
		exitFunc(1, "QUIT wasnt send correctly \n");
	}
	msg_code = recvMsg(sfd, buffer);
	if(msg_code != 221)
		exitFunc(1, "Failed to close connection \n");
	//close socket
	close(sfd);


	return 1;
}


/****************************************	MAIN 	****************************************/

int main(int argc, char *argv[]){


	optParser(argc,argv);
	//open input file containing autentization details
	log_file.open(lfile, ios::in);
	if (!log_file.is_open())	//throw error if file couldnt open
		exitFunc(1, "Login file couldn't be opened or is not valid\n");

	getLogInf(); //get login info
	//structure login now holds login informations

	//connect to server
	srvCommConnect();



	exitFunc(0,"everything went smoothly\n");
}