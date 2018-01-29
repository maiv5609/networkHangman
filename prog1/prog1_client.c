/* demo_client.c - code for example client program that uses TCP */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*------------------------------------------------------------------------
* Program: demo_client
*
* Purpose: allocate a socket, connect to a server, and print all output
*
* Syntax: ./demo_client server_address server_port
*
* server_address - name of a computer on which server is executing
* server_port    - protocol port number server is using
*
*------------------------------------------------------------------------
*/
int main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */
	int n; /* number of characters read */
	char board[1000]; /* buffer for data from the server */
	char guessBuf[1000]; /* buffer to store guesses from cilents */

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	if (port > 0) /* test for legal value */
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Connect the socket to the specified server. */
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

	/* Repeatedly read data from socket and write to user's screen. */
	//Default value
	uint8_t guesses = 1;
	int contacts = 0;
	char guess;
	int bufIdx = 0;
	guessBuf[0]= '\0';
	while (guesses > 0 && guesses != 255) {
		n = recv(sd, &guesses, sizeof(uint8_t),MSG_WAITALL);
		printf("client guesses: %i\n", guesses);
		//recieve board
		if(contacts == 0){
			contacts = guesses;
		}
		n = recv(sd, board, sizeof(board), 0);
		board[contacts] = '\0';
		printf("n is: %d\n", n);
		if(guesses != 0 && guesses != 255){
			printf("board is: %s\n", board);
			printf("Please insert your guess\n");
			printf("guessBuf[0]: %c\n", guessBuf[bufIdx]);
			if(guessBuf[bufIdx] == '\0' && guessBuf[bufIdx] != '\n'){
				printf("Made it\n");
				bufIdx = 0;
				fgets(guessBuf, sizeof(guessBuf), stdin); //fgetc only get c
				guess = guessBuf[bufIdx];
				bufIdx++;
			}
			else{
				guess = guessBuf[bufIdx];
				bufIdx++;
			}

			printf("guess is: %s\n", guessBuf);// gets user guess
			//printf("strlen is: %d\n\n", (int)strlen(guessBuf));

			send(sd, &guess, 1,0);
		}
	}
	dprintf(2, "exited\n");
	if(guesses == 255){
		printf("You win\n");
	}else{
		printf("You lost\n");
	}

	close(sd);

	exit(EXIT_SUCCESS);
}
