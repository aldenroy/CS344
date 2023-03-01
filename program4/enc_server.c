#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

int charToInt (char c){
	if (c == ' '){
		return 26;
	}
	else {
		return (c - 'A');
	}
	return 0;
}

char intToChar(int i){
	if (i == 26){
		return ' ';
	}
	else {
		return (i + 'A');
	}
}

void encrypt(char* message, char* key){ //encrypt a given message
	int i;
	char n;
	for (i=0; message[i] != '\n'; i++){
	  		char c = message[i];
	  		n = (charToInt(message[i]) + charToInt(key[i])) % 27;

	  		message[i] = intToChar(n);
	  }
	  message[i] = '\0';
	  return;
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
	// Clear out the address struct
	memset((char*) address, '\0', sizeof(*address)); 

	// The address should be network capable
	address->sin_family = AF_INET;
	// Store the port number
	address->sin_port = htons(portNumber);
	// Allow a client at any address to connect to this server
	address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
	int connectionSocket, charsRead, establishedConnectionFD, status;
	char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;
	socklen_t sizeOfClientInfo = sizeof(clientAddress);
	pid_t pid;

	// Check usage & args
	if (argc < 2) { 
		fprintf(stderr,"USAGE: %s port\n", argv[0]); 
		exit(1);
	} 
	
	// Create the socket that will listen for connections
	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket < 0) {
		error("ERROR opening socket");
	}

	// Set up the address struct for the server socket
	setupAddressStruct(&serverAddress, atoi(argv[1]));

	// Associate the socket to the port
	if (bind(listenSocket, 
			(struct sockaddr *)&serverAddress, 
			sizeof(serverAddress)) < 0){
		error("ERROR on binding");
	}

	// Start listening for connetions. Allow up to 5 connections to queue up
	listen(listenSocket, 5); 
	
	// Accept a connection, blocking if one is not available until one connects
	while(1){
		// Accept the connection request which creates a connection socket
		connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
		if(connectionSocket < 0){
			error("ERROR on accept");
		}
		pid = fork();
		switch(pid){
			case -1:{
				error("Hull Breach! Couldn't fork!\n");
			}
			case 0:{
				char buffer[1024];
				char* encryptedMessage[100000];
				char message[100000];
				char key[100000];
				int charsWritten = 0;
				memset(buffer, '\0', 1024); //CHECK FOR RIGHT CONNECTION
				charsRead = 0;
				while(charsRead == 0){
					charsRead = recv(establishedConnectionFD, buffer, 1023, 0); // Read the client's message from the socket					
				}

				if (charsRead < 0){
					error("ERROR reading from socket");
				}
				
				if(strcmp(buffer, "otp_enc") != 0){
					charsRead = send(establishedConnectionFD, "no", 2, 0);
					exit(2);
				}else{
					memset(buffer, '\0', 1024); //clear buffer
					charsRead = send(establishedConnectionFD, "yes", 3, 0); //SEND RESPONSE
					charsRead = 0;
					while(charsRead == 0){ //RECEIVE FILE LENGTH
						charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //RECEIVE FILE LENGTH
						//printf("%d\n", charsRead);

					}
					//printf("buffer on server: %s", buffer);
					int size = atoi(buffer);
					//printf("size of file on server: %d\n", size);

					charsRead = send(establishedConnectionFD, "cont", 4, 0);//continue
					charsRead = 0;
					int charsSent = 0;
					//printf("size: %d", size);
					while(charsRead < size){
						memset(buffer, '\0', 1024); //clear buffer
						charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //RECEIVE THE MESSAGES
						charsRead += charsSent;
						charsSent = 0;
						strcat(message, buffer);
						//printf("charsRead: %d\n", charsRead);
						memset(buffer, '\0', 1024); //clear buffer
						//charsWritten = send(establishedConnectionFD, "next", 4, 0); //ask for more
					}
					//printf("%d ", charsRead);
					//printf("%s\n", message);
					//charsWritten = send(establishedConnectionFD, "givekey", 7, 0);
					charsRead = 0;
					charsSent = 0;
					//printf("size: %d", size);
					while(charsRead < size){
						memset(buffer, '\0', 1024); //clear buffer
						charsSent = recv(establishedConnectionFD, buffer, sizeof(buffer)-1, 0); //RECEIVE THE MESSAGES
						charsRead += charsSent;
						charsSent = 0;
						strcat(key, buffer);
						//printf("charsRead: %d\n", charsRead);
						memset(buffer, '\0', 1024); //clear buffer
					}
					//printf("%s\n", message);
					//printf("%s", key);
					//printf("ENCRYPTED TEXT:\n\n\n\n\n");

					//encrypt
					encrypt(message, key);
					//printf("%s\n", message);
					memset(buffer, '\0', 1024); //clear buffer

					//SEND BACK TO CLIENT

					charsWritten = 0;
					while(charsWritten < size){
						memset(buffer, '\0', sizeof(buffer)); //clear out buffer
						charsWritten += send(establishedConnectionFD, message, sizeof(message), 0);
						memset(buffer, '\0', sizeof(buffer)); //clear out buffer						
					}	

					exit(0);

				}
			}
			default:{
				pid_t actualpid = waitpid(pid, &status, WNOHANG);
			}
		}
		printf("SERVER: Connected to client running at host %d port %d\n", 
							ntohs(clientAddress.sin_addr.s_addr),
							ntohs(clientAddress.sin_port));
		// Close the connection socket for this client
		close(establishedConnectionFD); 
	}
	// Close the listening socket
	close(listenSocket); 
	return 0;
}