#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <poll.h>

#define PORT "3490" 	// the port users will be connected to 
#define BACKLOG 10 	// How many connection the queue will hold

static const int MAX_BUFFER_SIZE = 4096;

void sigchld_handler(int s){
	// waitpid() might overwrite errno so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa){
	if(sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_listening_fd(){

	// Using getaddrinfo, look through appropriate places to open a recieving socket
	int sock_fd;
	int status;

	// This will be information about the selected node in the returned linked list
	struct addrinfo hints, *servinfo, *step; // will point to results
	struct sigaction sa;
	int yes = 1;
	int rv;
				   
	memset(&hints, 0, sizeof(hints));	// Set all values in hints to 0
	hints.ai_family = AF_UNSPEC;		// Unspecified whether IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// Socket type connections
	hints.ai_flags = AI_PASSIVE;		// handle IP for me
				
	// We use getaddrinfo to bind to the first port that fulfils our requirements
	// that is valid. We can dispose of it immediately after. 
	     
	if((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0){
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return -1;
	}

	// loop through returned linked list and look for valid port to bind fd to
	for(step = servinfo; step!=NULL; step = step->ai_next){
		if((sock_fd = socket(step->ai_family, step->ai_socktype, step->ai_protocol)) == -1){
			perror("server: socket");
			continue;
		}

		if( setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
			perror("setsockopt");
			exit(1);
		}

		if(bind(sock_fd, step->ai_addr, step->ai_addrlen) == -1){
			close(sock_fd);
			perror("server:bind");
			continue;

		}
		break;
	}

	freeaddrinfo(servinfo); // free up memory once finished using
	
	// check to see if failed to bind to address in linked list
	if(step == NULL){
		fprintf(stderr, "server:failed to bind\n");
		exit(1);
	}
	
	// attempt to listen()
	if(listen(sock_fd, BACKLOG) == -1){
		perror("listen");
		exit(1);
	}

	// reap all dead processes
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}

	return sock_fd;
}

void add_to_poll_fds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size){
	
	if(*fd_count == *fd_size){
		*fd_size *= 2;

		*pfds = (struct pollfd*)realloc(*pfds, sizeof(struct pollfd) * (*fd_size));
	}

	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN;

	(*fd_count)++;
}

void delete_from_poll_fds(struct pollfd pfds[], int i, int *fd_count){
	pfds[i] = pfds[*fd_count-1];

	(*fd_count)--;

}

// TODO - seperate sending messages into a method

int main(){

	int listener_fd;
	
	int new_fd;
	socklen_t addrlen;
	struct sockaddr_storage remote_addr;	// connectors address information		
	
	// TODO - add scalable buffer
	char buf[256]; //Buffer for client data

	char remoteIP[INET6_ADDRSTRLEN];
		       
	char s[INET6_ADDRSTRLEN];
	
	// TODO - scale up amount of connections allowed to fit DB conventions
	// Start off with room for 5 connections
	int fd_count = 0;
	int fd_size = 30;
	struct pollfd *pfds = (struct pollfd *)malloc(sizeof *pfds * fd_size);

	listener_fd = get_listening_fd();

	if(listener_fd == -1){
		fprintf(stderr, "error getting listening socket");
		exit(1);
	}
        
	pfds[0].fd = listener_fd;
	pfds[0].events = POLLIN; // REPORT AS READY TO LISTEN TOO, this will ensure new connections are listend to

	fd_count = 1;

  	printf("server: waiting for connections... \n");

	while(true){ // accept() connections and store them
		
		int poll_count = poll(pfds, fd_count, -1);

		if(poll_count == -1){
			perror("poll");
			exit(1);
		}

		// loop through connections looking for data to read
		for(int i = 0; i<fd_count; i++){
			
			// Founds a fd with polling flag enabled
			if(pfds[i].revents & POLLIN){
				
				// if the connection is the listener fd
				if(pfds[i].fd == listener_fd){
					
					// check for new connections
					addrlen = sizeof remote_addr;
					new_fd = accept(listener_fd,
						(struct sockaddr *)&remote_addr,
						&addrlen);

					if(new_fd == -1){
						perror("accept");
					} else {

						add_to_poll_fds(&pfds, new_fd, &fd_count, &fd_size);
						
						char msg[17] = "0013Hello World\0";
						
						ssize_t bytes_sent = send(new_fd,msg,17, 0);
						if (bytes_sent == -1) {
							perror("server:send()");
						}
	
						printf("pollserver: new connection from %s on "
							"socket %d\n",
							inet_ntop(remote_addr.ss_family,
								get_in_addr((struct sockaddr*)&remote_addr),
								remoteIP, INET_ADDRSTRLEN),new_fd);
	
					}
				}else{

					char head_buffer[4];
					uint32_t msgSize = 0;

					// Read 4 digits from buffer if exists (the header)
					ssize_t bytes_read = recv(pfds[i].fd, head_buffer, 4, 0);
					if (bytes_read == -1) {
						perror("server: recv");
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
						ssize_t msg_bytes_read = recv(pfds[i].fd, msg_Buf, msgSize, 0);

						if (msg_bytes_read != msgSize) {
							perror("server:recv error");
							continue;
						}
						msg_Buf[msgSize] = '\0';
						// msg_Buf holde de inpoot

						// TODO - 
						// Read a command from the client
						// Write data to a file or the equivalent that is used in a database

						// TODO -
						// Add a protocol for different commands
						// For example, the client msg 'header' says what command type it is, 
						// then it is sent to a function that handles that command type
						// After the command type might be other informaiton which is picked
						// out from the header and read sequentially 


						// TODO - IMPORTANT
						// before going whole hog on writing to a complicated filetype, just write to a plain
						// text file for now
						// TODO -
						// I am going to compeltely rip SQLites' filetype from them and steal it for myself
						// https://sqlite.org/fileformat2.html 
						// SQLite has a mega file that stores all the data and has a 100byte header that gives
						// all the information that is needed to use the mega file

						// TODO  
						// - parse input as SQL type commands
						// - commit log for commands applied to the db
						// - write to memtable
						// - flush from memtable to sstable (More permanent record)
						
						// TODO
						// Put some kind of encryption on the database files / potentially make own filetype
						
						// ANATOMY OF AN APACHE CASSANDRA SSTABLE
						// - Once new SStable has been written to, old one removed
						// - Data.db actual data
						//		rows are organised by partition. These partiions are sorted in token order (by a 
						//		hash of the partition key). Within a partition rows are stored in the order of their 
						//		clustering keys
						// 
						// - Index.db index from partition keys to positions in the data.db file, may include row index
						// - Filter.db bloom filter of the partition keys in the SSTable
						// - compressioninfo.db metadata about the offsets and lengths of compression chunks in data.db
						// - statistics.db stores metadata about the sstable including timestamps, tombstones, clustering
						//		keys compaction, repair, compression etc
						// - digest.crc32 a CRC-32 digest of the DAta.db file
						// - TOC.txt a plain text list of the component files for the SSTable


						// SSTables can be optionally compresssed using block-based compression

						// get - lookup records and return them in some kind of readable format?
						// 
						// set - find the location of records and then replace them as necessary
						// 
						// del - find the location of records, remove them and then clean up

					}


					// TODO - big list

					// Accept messages from connected sockets

					// Parse them in another part of the program (i.e SQL style commands to manipulate data)

					// Look up the next steps that would typically happen in a proper database system

					// TODO - COMMANDS ENTER QUEUE TO ALTER DATA KEPT ON SERVER (first come, first served
					// TODO - When command fails send failure message
					// TODO - When command successful send success message
				}
			}
		}
	}

	return 0;
}
