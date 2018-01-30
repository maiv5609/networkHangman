/* demo_server.c - code for example server program that uses TCP */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define QLEN 6 /* size of request queue */
int visits = 0; /* counts client connections */
int playHangman(int sd2, char* word); /* Main game function */

/*------------------------------------------------------------------------
* Program: demo_server
*
* Purpose: allocate a socket and then repeatedly execute the following:
* (1) wait for the next connection from a client
* (2) send a short message to the client
* (3) close the connection
* (4) go back to step (1)
*
* Syntax: ./demo_server port
*
* port - protocol port number to use
*
*------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	struct sockaddr_in cad; /* structure to hold client's address */
	int sd, sd2; /* socket descriptors */
	int port; /* protocol port number */
	int alen; /* length of address */
	int optval = 1; /* boolean value when we set socket option */
	char buf[1000]; /* buffer for string the server sends */



	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port word\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */
	sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

	port = atoi(argv[1]); /* convert argument to binary */
	if (port > 0) { /* test for illegal value */
		sad.sin_port = htons((u_short)port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	/* Bind a local address to the socket */
	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* Specify size of request queue */
	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

	/* Main server loop - accept and handle requests */
	while (1) {
		alen = sizeof(cad);
		if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}
		signal(SIGCHLD,SIG_IGN); //kill zombies/children
		pid_t p = fork();
		if( p < 0 ) {
			fprintf(stderr,"fork failed\n");
			return 1;
		} else if( p == 0 ) { //child
			char word[strlen(argv[2])];
			strcpy(word, argv[2]);
			playHangman(sd2, word);
		} else {	//parent

		}
	}

}


/* playHangman
* sd2   the socket
* word  a copy of the command line argument. The secret word.
*
* Contains the main server loop. Also contains the game logic for hangman.
* Enforces rules for multiple clients. Leads to either a win after getting all
* letters in the word or leads to a lose after running out of guesses. Then the
* socket is closed.
*/
int playHangman(int sd2, char* word) {
	char buf[1]; //letter that will contain the guess from the client
	char board[strlen(word) + 1];
	for (int i = 0; i < strlen(word); i++){
		board[i] = '_'; //fill with _
	}
	board[strlen(word) + 1] = '\0'; //add null terminator

	int n = 0; //number of bytes received from recv
	uint8_t guesses = strlen(word);
	while (guesses > 0 && strchr(board, '_')){
		//Send guesses and board
		send(sd2, &guesses,sizeof(uint8_t),0);
		send(sd2, board, strlen(board),0);
		int correct = 0; //flag for checking if guess is correct
		int repeat = 0; // flag for if guesses is repeated
		n = recv(sd2,buf,1,MSG_WAITALL); //receive a single character
		if (n <= 0){
			printf("recv failed\n");
		}
		//check for repeats
		for(int i = 0; i < strlen(board); i++){
			if(board[i] == buf[0]){
				repeat = 1;
			}
		}
		//check for correct letter
		for(int i = 0; i < strlen(board); i++){
			if(word[i] == buf[0]){
				board[i] = buf[0];
				correct = 1;
			}
		}
		//decrement guesses accordingly
		if (correct == 0 || repeat == 1){
			guesses--;
		}
	}

	if (strchr(board, '_') == '\0'){
		guesses = 255;
	}
	send(sd2, &guesses,sizeof(uint8_t),0);
	send(sd2, board,strlen(board),0);
	close(sd2);
}
