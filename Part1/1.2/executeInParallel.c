#include <stdio.h> 
#include <sys/types.h> 
#include <string.h>  
#include <unistd.h>  
#include <stdlib.h> 
#include <errno.h>   
#include <sys/wait.h> 
#include <sys/stat.h>
#include <fcntl.h> 
#include <errno.h> 
extern int errno; 


int executeCommand (char *cmd) {
	char* path = getenv("CS330_PATH");
	// printf("PATH :%s\n",(path!=NULL)? path : "getenv returned NULL");
	int space=0, i=0, j=0;

	//Exctracting the command name and flags into args to be send to the execv syscall
	for(int i=0;cmd[i];i++){
		if(cmd[i]==' ')space++;
	}
	// printf("%d\n", cname_size);
	char* args[space+2];
	args[space+1]=NULL;
	for(int i=0;i<space+1;i++){
		args[i] = (char*)malloc(8*sizeof(char));
	}
	i=0;
	char* token = strtok(cmd, " "); 
    while (token != NULL) { 
        args[i++]=token;
        token = strtok(NULL, " "); 
    } 
	// for(int i=0;i<space+1;i++){
	// 	printf("%s\n", args[i]);
	// }
	
	// Exctracting the paths to the executables

	i=0;
	int colon=0;
	while(path[i]){
		if(path[i++]==':')colon++;
	}
	// printf("%d\n", colon);
	char* paths[colon+1];
	for(int i=0;i<colon+1;i++){
		paths[i]=(char*)malloc(64*sizeof(char));
	}
	i=0;
	token = strtok(path, ":"); 
    while (token != NULL) { 
        paths[i++]=token;
        token = strtok(NULL, ":"); 
    } 
	// for(int i=0;i<colon+1;i++){
	// 	printf("%s\n", paths[i]);
	// }

	// Creating a child process and finding the executable in the paths recieved

	int flag=1;
	i=0;
	for(;i<colon+1;i++){
		char* temp1 = (char*)malloc(sizeof(path[i])+sizeof(args[0])+sizeof(char));//stores final path to the executable
		char* temp2 = (char*)malloc(sizeof(args[0])+sizeof(char));
		strcat(temp1, paths[i]);
		temp2[0]='/';
		strcat(temp1, strcat(temp2, args[0]));
		// printf("%s\n", temp1);
		struct stat stats;
		if (stat(temp1, &stats) == 0)
		{
			if(stats.st_mode & X_OK){
				flag=execv(temp1, args);
			}
			else {
				printf("UNABLE TO EXECUTE\n");
				return -1;
			}
		}
	}
	if(i==colon+1 || flag==-1){
		printf("UNABLE TO EXECUTE\n");
		return -1;
	}
	return flag;
}

int execute_in_parallel(char *infile, char *outfile)
{
	int fdin = open(infile, 00);
	int fdout = open(outfile, 02 | 0100, 0666);
	if(fdin == -1){
		printf("UNABLE TO EXECUTE\n");
		return -1;
	}
	else {
		char buf[1500];
		int byte_read = read(fdin, buf, 1500);
		// printf("%d\n", byte_read);
		char* cmds[51];
		for(int i=0;i<51;i++){
			cmds[i]=(char*)malloc(25*sizeof(char));
		}
		int i=0;
		char* token = strtok(buf, "\n"); 
		while (token != NULL) { 
			cmds[i++]=token;
			token = strtok(NULL, "\n"); 
		} 
		// printf("%d\n", i);
		int size=i;
		// for(int i=0;i<size;i++){
		// 	printf("%s\n", cmds[i]);
		// }
		for(int i=0;i<size;i++){
			pid_t pid;
			int status;
			pid = fork();
			if(pid==0){
				dup2(fdout, 1);
				// printf("%s\n", cmds[i]);
				if(executeCommand(cmds[i])==-1){
					return -1;
				};
			}
			else if(pid>0){
				while(wait(&status) != -1) ;//pick up zombie children
			}
			else {
				printf("UNABLE TO EXECUTE\n");
			}
		}
	}
	close(fdin);
	close(fdout);
	return 0;
}

int main(int argc, char *argv[])
{
	return execute_in_parallel(argv[1], argv[2]);
}
