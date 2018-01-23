#include "stdio.h"
#include "unistd.h"
#include "signal.h"

int main() {
	signal(SIGCHLD,SIG_IGN);
	pid_t p = fork();
	if( p < 0 ) {
		fprintf(stderr,"fork failed\n");
		return 1;
	} else if( p == 0 ) {
		fprintf(stderr,"Child says hello\n");
		sleep(5);
		fprintf(stderr,"Child says goodbye\n");
		return 0;
	} else {
		fprintf(stderr,"Parent says hello, my child is %u\n",p);
		sleep(10);
		fprintf(stderr,"Parent says goodbye\n");
		return 0;
	}
	return 0;	
}
