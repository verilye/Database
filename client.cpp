#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT "3490"

// Helper function to convert sockaddr address into human readable IPv4 or IPV6
void *get_in_addr(struct sockaddr *sa){
	
	// cast to either IPv4 or IP46 and return (different address types have different members)
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return  &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_listening_fd(char *argv[]){

	int sock_fd, status;
	char s[INET6_ADDRSTRLEN];

	// hints is the desired parameters of the connection fd
	// head will point to the first node in the linked list returned by getaddrinfo
	// step will be used to iterate throught he list
	
	struct addrinfo hints, *head, *step;
	// all values must be specified or 0
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((status = getaddrinfo(argv[1], PORT, &hints, &head)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}

	// loop through the resulting linked list to find a suitable port
	for(step = head; step !=NULL; step = step->ai_next){

		// Cases 
		// suitable socket - bind to it and exit loop
		if((sock_fd = socket(step->ai_family, step->ai_socktype, step->ai_protocol)) == -1){  
			// handle appropriate error for socket() output
			perror("client:socket");
			continue;
		}

		// connect
		if((connect(sock_fd, step->ai_addr, step->ai_addrlen) == -1)){
			close(sock_fd);
			perror("client:connect");
			continue;
		}

		break;
	}

	// if no socket appropriately bind(), exit program	
	if(step == NULL){
		fprintf(stderr, "client: failed to connect\n");
		exit(1);
	}
	
	// Convert IP address to human readable form
	inet_ntop(step->ai_family,get_in_addr((struct sockaddr *)step->ai_addr),s, sizeof(s));

	printf("Connecting to server %s\n", s);
	
	//Frees the linked list 
	freeaddrinfo(head);
	
	return sock_fd;

}


// Print text to command line
void printMsg(ssize_t length, char * buffer){

	buffer[length] = '\0';
	printf("%s",buffer);

}

// TODO: change this to accept sql type commands and accept input equal to the maximum
// 	data size
void handleUserInput(int sockfd){

	char buffer[1000];

	while(1){
		printf("User Input:");
		if(fgets(buffer, 1000, stdin)){		
			send(sockfd, buffer,strlen(buffer), 0);	
		}
	}
}

// Start a loop to connect to the server and send out input to the server
int main (int argc, char *argv[]){

	int sock_fd, numbytes;
	sock_fd = get_listening_fd(argv);

	int yes = 1;
	fd_set readfds;
	// TODO
	// remove fork, not really a need for live server streaming in a db
	pid_t pid = fork();

	int msgSize = 0;
	char buffer[4];


	if(pid<0){
		// if fork fails, exit program
		perror("Fork failed");
		exit(EXIT_FAILURE);

	}else if(pid == 0){
		// if child process, close socket and accept user input
		handleUserInput(sock_fd);

	}else{
		
		// if parent process, accept data from socket and update in real time
		while(1){

			// TODO
			// Variable buffer implementation by reading 4 digits from the front of data recieved

			// Read 4 digits from buffer if exists
			if(recv(sock_fd, buffer,4,0) == -1){
				
				perror("client: recv");
				continue;
			}
				

			// NOTE - apparently you can memcopy an integer from the buffer straight to
			// an integer pointer and that will convert the data type for you?
			memcpy(&msgSize, buffer, 4);
			char msgBuf[msgSize];

			// TODO - there is a problem here where messages are printed infinitely
			// Add safe code to accept the number of bytes from the buffer specified in a header,
			// otherwise reject

			// get size of the buffer returned by recv 
			ssize_t bytes_read = recv(sock_fd, msgBuf,msgSize-1,0);
			if(bytes_read > 0){		
				//print msg
				printMsg(bytes_read, msgBuf);
			}
	
			
		}
	}
	
	close(sock_fd);

	return 0;
}
