#include <stdio.h>
#include <stdlib.h>
#include <netdb.h> 
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

void error(const char *msg) {
	perror(msg);
	exit(1);
} 


void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname){
	
	// Clear out the address struct
	memset((char*) address, '\0', sizeof(*address)); 

	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);

	// Get the DNS entry for this host name
	struct hostent* serverHostInfo = gethostbyname("localhost"); 
	if (serverHostInfo == NULL) { 
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
		exit(0); 
	}
	// Copy the first IP address from the DNS entry to sin_addr.s_addr
	memcpy((char*) &address->sin_addr.s_addr, serverHostInfo->h_addr_list[0], serverHostInfo->h_length);
}

int main(int argc, char *argv[]) //where communication between client and server happens
{
	int socketFD, portNumber, charsWritten, charsRead, bytes;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[1024];
	char ciphertext[100000];
	if (argc < 3) { 
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
		exit(0); 
	} 																					  

	setupAddressStruct(&serverAddress, atoi(argv[3]), argv[1]);
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketFD < 0){
		error("CLIENT: ERROR opening socket");
	}
	int yes = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	//resusable socket using https://beej.us/guide/bgnet/html/#setsockoptman

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		error("CLIENT: ERROR connecting");
	}

	long length_key;
	long length_file;
	int char_rep;
	int count = 0;
	FILE* file = fopen(argv[1], "r");

    while (1) {
        char_rep = fgetc(file);

        if (char_rep == EOF || char_rep == '\n')
            break;
		if(!isupper(char_rep) && char_rep != ' '){
			error("File contains bad characters!\n");
		}
        ++count;
    }
	fclose(file);
	length_file = count;

	count = 0;
	file = fopen(argv[2], "r");
    while(1) {
		//get characters and check for bad ones
        char_rep = fgetc(file);

        if (char_rep == EOF || char_rep == '\n')
            break;
		if(!isupper(char_rep) && char_rep != ' '){
			error("File contains bad characters!\n");
		}
        ++count;
    }
	fclose(file);
	length_key = count;


	if(length_file > length_key){
		printf("Key is too short\n!");
		error("Key is too short!\n");
	}
	//send error if filelength longer than key
	
	char* msg = "otp_enc";
	charsWritten = send(socketFD, msg, strlen(msg), 0); 
	memset(buffer, '\0', sizeof(buffer));
	if (charsWritten < 0) {
		error("CLIENT: ERROR writing from socket");		
	}

	charsRead = 0;
	while(charsRead == 0){
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
	}

	if (charsRead < 0){
 		error("CLIENT: ERROR reading from socket");		
	}//error handling

	if(strcmp(buffer, "no") == 0){ //check if bad client
		fprintf(stderr, "Bad client\n");
		exit(2);
	}

	memset(buffer, '\0', sizeof(buffer)); 
	sprintf(buffer, "%d", length_file);

	charsWritten = send(socketFD, buffer, sizeof(buffer), 0);
	memset(buffer, '\0', sizeof(buffer));
	//send size and clear out buffer
	charsRead = 0;

	while(charsRead == 0){
		charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0);		
	}

	if(strcmp(buffer, "cont") == 0){
		int fd = open(argv[1], 'r');
		charsWritten = 0;
		while(charsWritten <= length_file){
			memset(buffer, '\0', sizeof(buffer));
			bytes = read(fd, buffer, sizeof(buffer)-1);
			charsWritten += send(socketFD, buffer, strlen(buffer), 0);
			memset(buffer, '\0', 1024); 
		}//clear buffer again

		fd = open(argv[2], 'r');
		charsWritten = 0;
		//start sending the text from the key
		while(charsWritten <= length_file){
			memset(buffer, '\0', sizeof(buffer));
			bytes = read(fd, buffer, sizeof(buffer)-1);
			charsWritten += send(socketFD, buffer, strlen(buffer), 0);
			memset(buffer, '\0', 1024); 
		}
	}
	memset(buffer, '\0', sizeof(buffer)); 


	charsRead = 0;
	int charsSent = 0;
	//received the messages and start adding to ciphertext
	while(charsRead < length_file){
		memset(buffer, '\0', 1024); 
		charsSent = recv(socketFD, buffer, sizeof(buffer)-1, 0);
		charsRead += charsSent;
		charsSent = 0;
		strcat(ciphertext, buffer);
		memset(buffer, '\0', 1024);
	}
	strcat(ciphertext, "\n");
	printf("%s", ciphertext);
	close(socketFD); // Close the socket
	return 0;
}

