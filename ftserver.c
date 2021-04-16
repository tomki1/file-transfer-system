/**************
Author: Kimberly Tom
Project 2: File Transfer System
Description: Server side code written in C.  This program can send a list of files in a server's directory to a client or send a file to a client.
This program uses TCP connection to transfer data.
CS372 Intro To Computer Networks
Last Modified: 11/28/19
**************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>


#define BACKLOG 10 // only hold 10 connections in queue http://beej.us/guide/bgnet/html/


// sockMake creates socket with the server information
// preconditions: needs serverInformation as an argument
// postconditions: returns a created socket. If there is an error with making the socket, prints error and program exits.
// with help from http://beej.us/guide/bgnet/html/#socket
int SockMake(struct addrinfo * servInformation) {
	int mySocket;
	if ((mySocket = socket((struct addrinfo *)(servInformation)->ai_family, servInformation->ai_socktype, servInformation->ai_protocol)) == -1){
		perror("error with making socket");
		exit(1);
	}
	
	return mySocket;	
}

// SockConnect connects socket to server
// preconditions: needs a socket and serverInf as argument
// postconditions: connects socket to server.  If there is an error with connecting socket, prints error and program exits.
// with help from http://beej.us/guide/bgnet/html/#connectman
void SockConnect(int sockfd, struct addrinfo * servInf){             
	if ((connect(sockfd, servInf->ai_addr, servInf->ai_addrlen)) == -1){
		perror("error with connecting socket");
		exit(1);
	}
}

// SockBind binds the socket to a port
// preconditions: needs a socket and servInformation as argument
// postconditions: binds socket.  If there is an error with binding socket, prints error and program exits.
// with help from http://beej.us/guide/bgnet/html/#bind
void SockBind(int sockfd, struct addrinfo *servInformation){ 
	if (bind(sockfd, servInformation->ai_addr, servInformation->ai_addrlen) == -1) {
		close(sockfd);
		perror("Error with binding socket");
		exit(1);
	}
}

// SockListen listens on socket
// preconditions: needs a socket as an argument
// postconditions: now listening for connections.  If there is an error with listening, prints error and program exits.
// with help from http://beej.us/guide/bgnet/html/#listen
void SockListen(int sockfd){ 
	if (listen(sockfd, BACKLOG) == -1){
		close(sockfd);
		perror("error with listening");
		exit(1);
	}
}

// getAddress will obtain a linked list of address information 
// preconditions: needs port number as an argument
// postconditions: returns serverInfo which points to a linked list of struct servInfo
// with help from http://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch
// reused aspects from project 1
struct addrinfo * getAddress(char* portNum){

	struct addrinfo hints; // provides hints regarding socket type
	struct addrinfo *servInfo; // this will point to the linked list of addrinfo result.  If there's an error, prints error and program terminates.
	int status;
	
	memset(&hints, 0, sizeof hints); // clear the struct
	hints.ai_family = AF_UNSPEC; // either IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream 
	hints.ai_flags = AI_PASSIVE;     // fill my IP

	if ((status = getaddrinfo(NULL, portNum, &hints, &servInfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}
	
	return servInfo;
}

// getAddressForTransfer will obtain a linked list of address information 
// preconditions: needs the clientIpAddress and the port number as an argument
// postconditions: returns serverInfo which points to a linked list of struct servInfo.  If there's an error, prints error and program terminates.
// with help from http://beej.us/guide/bgnet/html/#getaddrinfoprepare-to-launch
// reused aspects from project 1
struct addrinfo * getAddressForTransfer(char* clientIPAddress, char* portNum){      
	struct addrinfo hints; // provides hints regarding socket type
	struct addrinfo *servInfo; // this will point to the linked list of addrinfo result
	int status;

	memset(&hints, 0, sizeof hints); // clear the struct
	hints.ai_family = AF_UNSPEC; // either IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream
	
	
	if ((status = getaddrinfo(clientIPAddress, portNum, &hints, &servInfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		exit(1);
	}
	
	return servInfo;
}


// sendList sends a list of files in the server's directory to the client
// preconditions: needs clientIPaddress, port number, array holding file names, and the number of files as parameters
// postconditions: list of files in the server's directory sent to the client, one by one. 
// with help from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
void sendList(char * clientIPAddress, char * port, char ** listArray, int fileCount){ 
	// need to sleep for four seconds or connection will not work https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
	sleep(4); 
	// get the additional port number from the client. This port is used for transfering data
	struct addrinfo * serverInfoForTransfer = getAddressForTransfer(clientIPAddress, port);  
	// create socket with the address information
	int connSocket = SockMake(serverInfoForTransfer);
	// connect socket
	SockConnect(connSocket, serverInfoForTransfer);
	
	int z;
	z = 0;
	
	// send each file name in the array
	while (z < fileCount) {
		send(connSocket, listArray[z], 100 ,0);  
		z++;
	}
	
	// after we have finished looping through all the files in the array, send client phrase that indicates there are no more files to be sent
	// this needs to be done so the client can stop trying to receive more files from server
	// https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c helped me figure out that I could send a phrase back to client to indicate no more files
	char* completed = "noMore";
	send(connSocket, completed, strlen(completed), 0);
	
	close(connSocket); // close the connection
	freeaddrinfo(serverInfoForTransfer); // free this structure
}

// sendText sends a server file to the client
// preconditions: needs clientIPaddress, port number used for transfering data and client's desired filename as parameters
// postconditions: File sent to the client, in streams of text. If error, show error and exit program.
// with help from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
// with help from https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
void sendText(char* clientIPAddress, char * port, char * filename){                
	// need to sleep for four seconds or connection will not work https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
	sleep(4); // sleep for four seconds
	// get the additional port number from the client. This port is used for transfering data
	struct addrinfo * serverInfoForTransfer = getAddressForTransfer(clientIPAddress, port);  
	// create socket with the address information	
	int connSocket = SockMake(serverInfoForTransfer);  
	// connect socket
	SockConnect(connSocket, serverInfoForTransfer);
	
	// array to hold text of file
	char fileText[3000];          
    // clear the text array                                         
	memset(fileText, 0, sizeof(fileText));
	
	// variable to hold phrase that marks no more texts in file left to send
	// https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c helped me figure out that I could send a phrase back to client to indicate no more text
	char * noMoreText = "no!More!Text";
	
	// This portion of code regarding reading bytes and sending bytes to client is borrowed from https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
	// open file for reading only
	int fileObj = open(filename, O_RDONLY);  
	
	// keep looping because it may take multiple calls to read all the data
	while (1) {
		
		// read bytes from the file and store in readBytes
		int readBytes = read(fileObj, fileText, sizeof(fileText) - 1);
		// if no bytes were read, then there is no more bytes in this file to read, so we can break out of this
		if (readBytes == 0) 
			break;
		
		// if we have readBytes of less than 0, then there was an error
		if (readBytes < 0) {
			perror("Error with reading bytes from file");
			exit(1);
		}

		// create variable to point to where we left off writing
		void* pointLeftOff = fileText;
		// while there are still bytes being read, send these bytes to client through socket
		while (readBytes > 0) {
			int BytesSent = send(connSocket, pointLeftOff, sizeof(fileText),0);
			// if the bytes written variable is less than 0, there was an error. exit program
			if (BytesSent < 0) {
				perror("Error with reading bytes from file");
				exit(1);
			}
			// update readBytes by subtracting the bytes we have just sent. readBytes is what we still have left to read
			readBytes = readBytes - BytesSent;
			// update the point we left off to the old point we left off plus the new bytes we have just sent
			pointLeftOff = pointLeftOff + BytesSent;
		}
		
		// clear the text array
		memset(fileText, 0, sizeof(fileText));
	}
	
	// send phrase that marks no more text to send to client
	send(connSocket, noMoreText, strlen(noMoreText),0);
	
	
	close(connSocket); // close the connection
	freeaddrinfo(serverInfoForTransfer); // free this structure
}

// acceptClient accept connection with client and processes client's request
// preconditions: takes the new socket as a parameter
// postconditions: determines whether client wanted to receive a list of files in directory or get a file.
// with help from http://beej.us/guide/bgnet/html/#sendrecv
// with help from https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc
// with help from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c 
// with help from https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
void acceptClient(int new_fd){	            
	char * hasError = "1";
	int hasErrorLength = strlen(hasError);
	char * noError = "0";
	int noErrorLength = strlen(noError);
	int charBuffer = 100; 
	char flip[charBuffer]; // array to hold client's flip server name
	char port[charBuffer]; // port number array
    char clientRequest[charBuffer]; // array to hold clientRequest
    char clientIPAddress[charBuffer]; // array for IP address of client
	char nameOfFile[charBuffer]; // array to hold the name of the file we will be receiving from the client	
  
	// clear the port array
	memset(port, 0, sizeof(port));  
	// clear the flip array
	memset(flip, 0, sizeof(flip)); 	
	// clear the clientRequest array
	memset(clientRequest, 0, sizeof(clientRequest)); 
	// clear the ip address array
	memset(clientIPAddress, 0, sizeof(clientIPAddress));
	// clear the nameOfFile array
	memset(nameOfFile, 0, sizeof(nameOfFile));
		
	// receive the port number from the client
	recv(new_fd, port, sizeof(port)-1, 0);
	// send ok message to the client
	send(new_fd, noError, noErrorLength, 0);
	
 	// receive the flip	server from the client 
	recv(new_fd, flip, sizeof(flip)-1, 0);
	// send ok message to the client
	send(new_fd, noError, noErrorLength, 0);
	
	
	// receive the request from the client
	recv(new_fd, clientRequest, sizeof(clientRequest)-1, 0);
	// send ok message to the client
	send(new_fd, noError, noErrorLength,0);
	
	
	// receive the ip address from the client
	recv(new_fd, clientIPAddress, sizeof(clientIPAddress)-1,0);
	
	printf("connection from %s\n", flip);
    
	int max_size = 600;
	// create an array to hold file names in the directory
	// with help from https://stackoverflow.com/questions/5935933/dynamically-create-an-array-of-strings-with-malloc
	char ** directoryList;
	directoryList = malloc(max_size*sizeof(char*));
	int elements;
	for (elements = 0; elements < max_size; elements++) {
		directoryList[elements] = malloc(100*sizeof(char));
	}
	
	// clear the directory list
	for (elements = 0; elements < max_size; elements++) {
		memset(directoryList[elements], 0 ,sizeof(directoryList[elements]));
	}
		
	// open directory and fill directory array with file names
	// with help from https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/
	struct dirent * d;
	// open the directory
	DIR *dr = opendir(".");
	
	// number of files in directory
	int fileCount = 0;

		// while we are not at the end of the list of files, copy file name into the directory array
		while ((d = readdir(dr)) != NULL){
			{
			strcpy(directoryList[fileCount], d->d_name);
			fileCount++;
			}
		}
		
		// close the directory
		closedir(dr);
		
		
	// if theclient requests the directory, send directory
	// idea of checking whether client's command is for a list is from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c 
	if (strcmp(clientRequest,"list") == 0){              
		
		// if server got a request for list, send no error
		send(new_fd, noError, noErrorLength,0);
		
		printf("List directory has been requested on port %s\n", port);
		printf("Sending list of files in directory to %s:%s \n", flip, port);
		
			
		sendList(clientIPAddress, port, directoryList, fileCount);
		
	}
	
	
	// if the client requests a file, send file
	// idea of checking whether client's command is for a get is from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c 
	else if (strcmp(clientRequest,"get") == 0) {
		// if server got a request for get, send no error
		send(new_fd, noError, noErrorLength,0);
		printf("File requested.\n");
		
		// receive the name of the file the client is requesting
		recv(new_fd, nameOfFile, sizeof(nameOfFile)-1, 0);
		printf("The client requested to receive %s on port %s.\n", nameOfFile, port);
		
		
		// check to see if the file is in the directory
		int fileExists = 0;               
		int b;
		b = 0;

		// loop through files, if there is a match with nameOfFile, fileExists updated to true
		
		while (b < fileCount) {
			if(strcmp(directoryList[b], nameOfFile) == 0){
				fileExists = 1;
			}
				b++;
		}
		
		// variables to hold phrases for whether a file is present or not present
		char * fileThere = "fileThere";
		char * fileNotThere = "fileNotThere";
		
		// if file exists
		if (fileExists == 1) {
			// send message to client informing file exists
			send(new_fd, fileThere, strlen(fileThere), 0);
			printf("Sending %s to %s:%s.\n", nameOfFile, flip, port);	

			
			// send the file and its text to client
			sendText(clientIPAddress, port, nameOfFile);
			
		}
		
		// if file does not exist, inform client
		if (fileExists == 0) {
			send(new_fd, fileNotThere, strlen(fileNotThere), 0);
			printf("The file %s could not be found. Sending error message to %s:%s\n", nameOfFile, flip, port);	
			
		}
	}
	
	// if server did not receive a list or get request, inform client of invalid request
	else {
		send(new_fd, hasError, hasErrorLength, 0);
		printf("Got invalid clientRequest.\n");
	}

	// free each item in the directoryList
	int y;
	for (y = 0; y < max_size; y++){
		free(directoryList[y]);
	}
	// free the directoryList array itself
	free(directoryList);
	
	
	printf("Action completed.\n");
}


// acceptingClient accept connection with client
// preconditions: takes the first socket as a parameter
// postconditions: client's connection is accepted.  If there's an error, print error and exit program.
// with help from http://beej.us/guide/bgnet/html/#acceptthank-you-for-calling-port-3490.
void acceptingClient(int sockfd) {
	struct sockaddr_storage their_addr;	
    socklen_t sizeOfAddress;	
	int new_fd;
	
	// this is the main accept loop
	while(1){		
		sizeOfAddress = sizeof(their_addr);		
		new_fd = accept(sockfd, (struct addrinfo *)&their_addr, &sizeOfAddress);
		
		if(new_fd == -1){			
			perror("error with accept");
			continue;
		}

		acceptClient(new_fd); // accept a new connection from a client
		close(new_fd); // close connection
	}
}





int main(int argc, char *argv[]) 
{
	
	// if the user did not enter 2 arguments including the program name, show error and exit
	if (argc != 2) {
	    fprintf(stderr,"error: you didn't enter the correct number of arguments. Format: 'ftserver <SERVER_PORT>'\n");
	    return 1;
	}

	
	struct addrinfo* serverInformation = getAddress(argv[1]); // get the server information
	int sockfd = SockMake(serverInformation); // create socket with the server information
	SockBind(sockfd, serverInformation); // bind the socket to a port
	SockListen(sockfd); // listen on socket
	printf("Server open on %s\n", argv[1]);
	acceptingClient(sockfd); // now accepting connections
	freeaddrinfo(serverInformation); // free this structure
	
	
    
}
