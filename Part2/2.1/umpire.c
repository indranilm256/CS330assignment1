#include <stdio.h> 
#include <sys/types.h> 
#include <string.h>  
#include <unistd.h>  
#include <stdlib.h> 
#include <errno.h>   
#include <sys/wait.h> 
#include <sys/stat.h>
#include <time.h>

#define BUFSIZE 	2
#define STDIN 		0
#define STDOUT 		1

#define ROCK 		0 
#define PAPER 		1 
#define SCISSORS	2 

#define TOTAL_MOVES	3


int main(int argc, char* argv[]) {
	if(argc<3){
		printf("usage: ./umpire player1 player2");
		exit(0);
	}
	else {
		char* player1 = argv[1];
		char* player2 = argv[2];
		// printf("%s, %s\n", player1, player2);
		int p1[2], p2[2]; //pipe p1 is to send/read data from umpire to player, p2 is to send/read data from player to umpire
		if (pipe(p1) < 0) 
			exit(-1); 
		if (pipe(p2) < 0)
			exit(-1);
		// printf("%d, %d, %d, %d\n", p1[0], p1[1], p2[0], p2[1]);
		char moves1[10], moves2[10];
		pid_t pid = fork();
		char* outbuf = "GO\0";
		if(pid < 0 ){
			exit(-1);
		}
		else if(pid == 0 ){// in the child process
			close(p1[1]);//close writing end of pipe 1
			dup2(p1[0], STDIN);// the child now takes input from the reading end of pipe 1
			close(p1[0]);
			close(p2[0]);// close reading end of pipe 2
			dup2(p2[1], STDOUT);// the child now writes into the writing end of pipe 2
			close(p2[1]);
			char* args[] = {"player1", NULL}; 
			execv(player1, args);
		}
		else {// in the main process
			char inbuf1[BUFSIZE];
			close(p1[0]);//close reading end of first pipe
			close(p2[1]);//close writing  end of second pipe
			for(int i=0;i<10;i++){
				write(p1[1], outbuf, 2);//write into first pipe
				int n;
				if(-1 != (n = read(p2[0], inbuf1, 2))){//read byte from player 1 stdout
					// printf("Number recieved from player 1 is:%s\n", inbuf1);
					moves1[i]=inbuf1[0];
				}
				else {
					exit(-1);
				}
			}
			close(p1[1]);
			close(p2[0]);//close reading end of first pipe
			wait(NULL);// wait for child process to finish
		}
		if (pipe(p1) < 0) 
			exit(-1); 
		if (pipe(p2) < 0)
			exit(-1);
		// printf("%d, %d, %d, %d\n", p1[0], p1[1], p2[0], p2[1]);
		pid = fork();
		if(pid < 0 ){
			exit(-1);
		}
		else if(pid == 0 ){// in the child process
			close(p1[1]);//close writing end of pipe 1
			dup2(p1[0], STDIN);// the child now takes input from the reading end of pipe 1
			close(p1[0]);
			close(p2[0]);// close reading end of pipe 2
			dup2(p2[1], STDOUT);// the child now writes into the writing end of pipe 2
			close(p2[1]);
			char* args[] = {"player2", NULL}; 
			execv(player2, args);
		}
		else {// in the main process
			char inbuf2[BUFSIZE];
			close(p1[0]);//close reading end of first pipe
			close(p2[1]);//close writing  end of second pipe
			for(int i=0;i<10;i++){
				write(p1[1], outbuf, 2);//write into first pipe
				int n;
				if(-1 != (n = read(p2[0], inbuf2, 2))){//read byte from player 1 stdout
					// printf("Number recieved from player 2 is:%s\n", inbuf2);
					moves2[i]=inbuf2[0];
				}
				else {
					exit(-1);
				}
			}
			close(p1[1]);
			close(p2[0]);//close reading end of first pipe
			wait(NULL);// wait for child process to finish
		}
		// for(int i=0;i<10;i++){
		// 	printf("%c ", moves1[i]);
		// }
		// for(int i=0;i<10;i++){
		// 	printf("%c ", moves2[i]);
		// }
		int score1 = 0, score2 = 0;
		for(int i=0;i<10;i++){
			if((moves1[i]=='1' && moves2[i]=='0') || (moves1[i]=='2' && moves2[i]=='1') || (moves1[i]=='0' && moves2[i]=='2')){
				score1++;
			}
			else if((moves2[i]=='1' && moves1[i]=='0') || (moves2[i]=='2' && moves1[i]=='1') || (moves2[i]=='0' && moves1[i]=='2')){
				score2++;
			}
		} 
		printf("%d %d", score1, score2);
	}
	return 0;
}
