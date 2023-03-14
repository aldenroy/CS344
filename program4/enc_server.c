#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
	perror(msg);
	exit(1);
} 

char convert_int(int x){
	if (x == 26){
		return ' ';
	}
	else {
		return (x + 'A');
	}
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

int convert_char (char x){
	if (x == ' '){
		return 26;
	}
	else {
		return (x - 'A');
	}
	return 0;
}

int main(int argc, char *argv[]){
	int connectionSocket, charsRead, connection, status;
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
		sizeOfClientInfo = sizeof(clientAddress); //size of address for client
		connection = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
		if (connection < 0) error("ERROR on accept");

		//create new process with fork when connection is made.
		pid = fork();
		switch (pid){
			case -1:{
				error("Hull Breach! Couldn't fork!\n");
			}
			case 0:{
				char buffer[1024];
				char message[100000];
				char key[100000];
				int write_total = 0;
				memset(buffer, '\0', 1024);
				charsRead = 0;
				
				while(charsRead == 0){
					charsRead = recv(connection, buffer, 1023, 0);
					//checks clients message					
				}

				if(charsRead < 0){
					error("ERROR reading from socket");
				} 
				if(strcmp(buffer, "otp_enc") != 0){
					charsRead = send(connection, "no", 2, 0);
					exit(2);
				}else{
					memset(buffer, '\0', 1024);
					charsRead = send(connection, "yes", 3, 0);
					//send response back
					charsRead = 0;
					while(charsRead == 0){
						charsRead = recv(connection, buffer, sizeof(buffer)-1, 0);
						//get file lengths to check
					}
					charsRead = send(connection, "cont", 4, 0);
					charsRead = 0;
					int sent_total = 0;					
					
					int size = atoi(buffer);

					while(charsRead < size){
						memset(buffer, '\0', 1024); 
						sent_total = recv(connection, buffer, sizeof(buffer)-1, 0);
						charsRead += sent_total;
						sent_total = 0;
						strcat(message, buffer);
						memset(buffer, '\0', 1024);
						//receives messages needed and clears buffer like example code says.
					}
					charsRead = 0;
					sent_total = 0;

					while(charsRead < size){
						memset(buffer, '\0', 1024);
						sent_total = recv(connection, buffer, sizeof(buffer)-1, 0);
						charsRead += sent_total;
						sent_total = 0;
						strcat(key, buffer);
						memset(buffer, '\0', 1024); //clear buffer
					}

					//same situation as above but for the key instead of the message this time.
					//encryption step
					int i;
					char curr;
					for (i=0; message[i] != '\n'; i++){
						curr = (convert_char(message[i]) + convert_char(key[i])) % 27;

						message[i] = convert_int(curr);
					}
					message[i] = '\0';
					memset(buffer, '\0', 1024);
					write_total = 0;

					while(write_total < size){
						memset(buffer, '\0', sizeof(buffer));
						write_total += send(connection, message, sizeof(message), 0);
						memset(buffer, '\0', sizeof(buffer));					
					}	

					exit(0);

				}
			}
			default:{
				pid_t actualpid = waitpid(pid, &status, WNOHANG);
			}
		}
		close(connection);//Close the connection socket for this client
	}
	close(listenSocket); //Close the listening socket
	return 0;
}