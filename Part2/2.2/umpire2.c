#include "gameUtils.h"
#include <stdio.h> 
#include <sys/types.h> 
#include <string.h>  
#include <unistd.h>  
#include <stdlib.h> 
#include <errno.h>   
#include <sys/wait.h> 
#include <sys/stat.h>
#include <fcntl.h> 
#include <time.h>
#include <stdbool.h> 
#define BUFSIZE 	2
#define STDIN 		0
#define STDOUT 		1

#define ROCK 		0 
#define PAPER 		1 
#define SCISSORS	2 

#define TOTAL_MOVES	3


int getWalkOver(int numPlayers); // Returns a number between [1, numPlayers]

struct player{
	int id;
	int active;
	int paired;
	char path[100];
};

struct pair{
	struct player p1;
	struct player p2;
};

int referee(int rounds, char* path1, char* path2){
	char* player1 = path1;
	char* player2 = path2;
	// printf("%s, %s\n", player1, player2);
	int p1[2], p2[2]; //pipe p1 is to send/read data from umpire to player, p2 is to send/read data from player to umpire
	if (pipe(p1) < 0) 
		exit(-1); 
	if (pipe(p2) < 0)
		exit(-1);
	// printf("%d, %d, %d, %d\n", p1[0], p1[1], p2[0], p2[1]);
	char moves1[rounds], moves2[rounds];
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
		for(int i=0;i<rounds;i++){
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
		for(int i=0;i<rounds;i++){
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
	// for(int i=0;i<rounds;i++){
	// 	printf("%c ", moves1[i]);
	// }
	// for(int i=0;i<rounds;i++){
	// 	printf("%c ", moves2[i]);
	// }
	int score1 = 0, score2 = 0;
	for(int i=0;i<rounds;i++){
		if((moves1[i]=='1' && moves2[i]=='0') || (moves1[i]=='2' && moves2[i]=='1') || (moves1[i]=='0' && moves2[i]=='2')){
			score1++;
		}
		else if((moves2[i]=='1' && moves1[i]=='0') || (moves2[i]=='2' && moves1[i]=='1') || (moves2[i]=='0' && moves1[i]=='2')){
			score2++;
		}
	} 
	// printf("%d %d\n", score1, score2);
	return score1-score2 ;
}

int main(int argc, char *argv[])
{
	int N;
	char* path ;
	if(argc == 2){
		N=10;
		path=argv[1];
	}
	else {
		N=atoi(argv[2]);
		path=argv[3];
	}
	// printf("%d, %s\n", N, path);
	int fdin = open(path, 00);
	if(fdin == -1){
		printf("UNABLE TO EXECUTE\n");
		exit(-1);
	}
	else {
		char buf[100000];
		int byte_read = read(fdin, buf, 100000);
		// printf("%d\n", byte_read);
		char* token = strtok(buf, "\n"); 
		int P=atoi(token);
		// printf("P=%d\n", P);
		token = strtok(NULL, "\n");
		struct player players[P];
		int i=0;
		while (token != NULL) { 
			players[i].id=i;
			players[i].active=1;
			players[i].paired=0;
			strcpy(players[i].path, token);
			i++;
			token = strtok(NULL, "\n"); 
		} 
		// for(int i=0;i<P;i++){
		// 	printf("id=%d, path=%s\n", players[i].id, players[i].path);
		// }
		// for(int i=0;i<P;i++){
		// 	printf("p%d ", players[i].id);
		// }
		// printf("\n");
		int active=P;
		while(active){
			for(int i=0, j=0;i<P;i++, j++){
				if(players[i].active){
					printf("p%d", players[i].id);
					if(j<active && active!=1)printf(" ");
					j++;
				}
			}
			if(active>1){
				printf("\n");
			}
			// printf("active=%d\n", active);
			int walker=-1;
			if(active%2){
				walker=getWalkOver(active)-1;
			}
			for(int i=0;i<P;i++){//set paired of each to 0 initially
				if(players[i].active==1){
					players[i].paired=0;
				}
			}
			struct pair pairs[active/2];
			int j=0;
			for(int i=0;i<P && j<active;i++){
				if(players[i].active==1 && players[i].paired==0 && i!=walker){
					if(j%2==0){
						players[i].paired=1;
						pairs[j/2].p1=players[i];
						// printf("X%d", j);
					}
					else {
						players[i].paired=1;
						pairs[j/2].p2=players[i];
						// printf("Y%d", j);
					}
					j++;
				}
			}
			// for(int i=0;i<active/2;i++){//print pairs
			// 	printf("(%d, %d);", pairs[i].p1.id, pairs[i].p2.id);
			// }
			// printf("\n");
			for(int i=0;i<active/2;i++){
				int diff = referee(N, pairs[i].p1.path, pairs[i].p2.path);
				if(diff>=0){
					players[pairs[i].p2.id].active=0;
				}
				else {
					players[pairs[i].p1.id].active=0;
				}
			}
			active/=2;
			if(walker!=-1 && active!=0)active++;
		}
	}
	return 0;
}
