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
#define MAXDATASIZE 100 

// Helper function to convert sockaddr address into human readable IPv4 or IPV6
void *get_in_addr(struct sockaddr *sa){
	
	// cast to either IPv4 or IP46 and return (different address types have different members)
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return  &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void printMsg(ssize_t length, char * buffer){

	buffer[length] = '\0';
	printf("%s",buffer);

}

void handleUserInput(int sockfd){

	char buffer[MAXDATASIZE];

	while(1){
		printf("User Input:");
		if(fgets(buffer, MAXDATASIZE, stdin)){		
			send(sockfd, buffer,strlen(buffer), 0);	
		}
	}
}

int main (int argc, char *argv[]){

	
	// Get addr info in a linked list
	// loop through and select socket
	// bind() and then free up addrinfo linked list
	// listen() for user input 

	int status, sockfd, numbytes;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];

	// hints is the desired parameters
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
		if((sockfd = socket(step->ai_family, step->ai_socktype, step->ai_protocol)) == -1){
			// handle appropriate error for socket() output
			perror("client:socket");
			continue;
		}

		// connect
		if((connect(sockfd, step->ai_addr, step->ai_addrlen) == -1)){
			close(sockfd);
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
	
	// TODO: Create function that feeds an IPv4 or IPv6 value into the function
	// Convert IP address to human readable form
	// 2nd argument is the unreadable IP address, s is a buffer
	inet_ntop(step->ai_family,get_in_addr((struct sockaddr *)step->ai_addr),s, sizeof(s));

	printf("Connecting to server %s\n", s);
	
	//Frees the linked list 
	freeaddrinfo(head);

	// An fd_set is used here for I/O multiplexing. We are going to swap
	// between user input and readin from the server using select()
	fd_set readfds;
	pid_t pid = fork();

	if(pid<0){
		// if fork fails, exit program
		perror("Fork failed");
		exit(EXIT_FAILURE);

	}else if(pid == 0){
		// if child process, close socket and accept user input
		handleUserInput(sockfd);

	}else{
		// if parent process, accept data from socket and update in real time
		char buffer[MAXDATASIZE];
		while(1){
	
			// get size of the buffer returned by recv 
			ssize_t bytes_read = recv(sockfd, buffer, MAXDATASIZE-1,0);
			if(bytes_read > 0){		
				//print msg
				printMsg(bytes_read, buffer);
			}
		}
	}

	

	
	close(sockfd);

	return 0;
}
