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

int get_listening_fd(){

	int sock_fd, status;
	char s[INET6_ADDRSTRLEN];

	// socket information stored in a struct
	struct addrinfo hints, *head, *step;
	// all values must be specified or 0
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((status = getaddrinfo(NULL, PORT, &hints, &head)) != 0){
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
void printMsg(char * buffer){

	printf("%s\n",buffer);

}

void handleUserInput(int sockfd){

	// Reads the size of the message and appends its length as a 4 byte header
	int strSize = 0;
	char buffer[1000];

	printf("user input:");
	if ((strSize = ((char)strlen(fgets(buffer+4, 1000, stdin))))) {

		//TODO - replace with MAX BUFFER SIZE, should be base 10 hopefully
		int v = 1000;
		for (int i = 0; i < 4; i++) {

			int temp = (strSize / v);
			if (temp < 1) {

				buffer[i] = '0';
			}
			else {

				buffer[i] = ('0' + temp);

			}

			v /= 10;

		}

		ssize_t bytes_sent = send(sockfd, buffer, strlen(buffer), 0);
		if (bytes_sent == -1) {
			perror("client:send()");
		}
	}
}

// Start a loop to connect to the server and send out input to the server
int main (int argc, char *argv[]){

	// TODO - decide on the size of queries allowed and make sure max buffer that size

	int sock_fd = get_listening_fd();
	int yes = 1;
	
	while (1) {

		char head_buffer[4];
		uint32_t msgSize = 0;

		// Read 4 digits from buffer if exists (the header)
		ssize_t bytes_read = recv(sock_fd, head_buffer, 4, 0);
		if (bytes_read == -1) {
			perror("client: recv");
			continue;
		}
		else if (bytes_read == 0) {
			continue;
		}
		
		if (bytes_read >= 4) {

			// atoi works by converting to the respective ascii character  = c + '0' etc 
			msgSize = atoi(head_buffer);

			// Read the "body" of the message
			char msg_Buf[msgSize + 1];
			ssize_t msg_bytes_read = recv(sock_fd, msg_Buf, msgSize, 0);
			
			if (msg_bytes_read != msgSize) {
				perror("client:recv error");
				continue;
			}
			msg_Buf[msgSize] = '\0';
			
			printMsg(msg_Buf);
			handleUserInput(sock_fd);

		}

		
	}

	
	close(sock_fd);

	return 0;
}
