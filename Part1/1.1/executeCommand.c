#include <stdio.h> 
#include <sys/types.h> 
#include <string.h>  
#include <unistd.h>  
#include <stdlib.h> 
#include <errno.h>   
#include <sys/wait.h> 
#include <sys/stat.h>

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

	pid_t pid;
	pid = fork();

	int flag=1;
	if(pid == -1){//pid -1 means error in fork syscall
		printf("UNABLE TO EXECUTE\n");
		exit(-1);
	}
	else if(pid == 0){//this portion is executed in the child process
		int i=0;
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
	}
	else {// in the parent process
	    int status; 
		waitpid(pid, &status, 0); 
		if ( WIFEXITED(status) ) 
		{ 
			int exit_status = WEXITSTATUS(status);         
			// printf("Exit status of the child was %d\n", exit_status); 
			return exit_status;
		} 
	}
	return flag;
}

int main (int argc, char *argv[]) {
	return executeCommand(argv[1]);
}
