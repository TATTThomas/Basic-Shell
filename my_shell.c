#ifndef _POSIX_C_SOURCE
	#define  _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>

int com_num, no;
char record[16][100];

char *command_read(){

	char* command;
	command = (char*)malloc(100 * sizeof(char));
	ssize_t bufsize = 0;
	getline(&command, &bufsize, stdin);
	char tmp[6];
	strncpy(tmp, command, 6);
	no++;
	if(strcmp(tmp, "replay") != 0){
		for(int i = 14; i >= 0; i--){
			strcpy(record[i + 1], record[i]);
			//fputs(record[i], stdout);
			//printf("\n");
		}
		
		strcpy(record[0], command);
    }
    //fputs(record[15], stdout);
    //printf("\n");
	return command;

}

#define tok_remove " \t\r\n\a"
char*** get_args(char *command){

	int buf = 100, idx = 0, id = 0, a = 0, b = 0;
	com_num = 0;

	char ***tokens = malloc(buf * sizeof(char**));
	
	for(int i = 0; i < buf; i++){
		tokens[i] = malloc(10 * sizeof(char*));
	}
	char *token;
    bool check = false;
	token = strtok(command, tok_remove);
	while(token != NULL){

        check = true;
        
        if(strcmp(token, "|") == 0 || strcmp(token, ">") == 0 || strcmp(token, "<") == 0){
        	check = false;
        	a = 0;
        	if(strcmp(token, ">") == 0 || strcmp(token, "<") == 0){
        		a++;
        	}
        }
        if(check){
        	a++;
        }
        
        if(a > 1 && check == true){
        	tokens[idx - 1][id] = token;
            id++;
        }
        else{
	        if(idx != 0){
	        	tokens[idx - 1][id] = NULL;
	        }
	        id = 0;
	        tokens[idx][id] = token;
	        id++;
	        idx++;
	        com_num++;
		}
		token = strtok(NULL, tok_remove);
	}
	tokens[idx][0] = NULL;
  	return tokens;
}

void help_f(void){
    printf("=============================================================\n");
    printf("Welcome to my shell.\n");
    printf("Type prgram name and arguments, and press enter.\n");
    printf("\n");
    printf("The following are built in:\n");
    printf("1. help: show all build-in func info\n");
    printf("2. cd: change directory\n");
    printf("3. echo: echo the string to std output\n");
    printf("4. record: show last 16 cmds you type in\n");
    printf("5. replay: re-executed the cmd show in record\n");
    printf("6. mypid: find and print process-ids\n");
    printf("7. exit: exit shell\n");
    printf("=============================================================\n");
}

void background(char ***args){
	int i = 0;
    int num_of_pipes = 0;
    bool have_rediout = false, have_rediin = false, have_pipe = false;
    pid_t pid, back;
    back = fork();
    if(back == -1){
    	perror("fork");
		exit(1);
	}
	else if(back == 0){
		while (args[i][0] != NULL){
		    if(strcmp(args[i][0], "|") == 0){
		        num_of_pipes++;
		        have_pipe = true;
		    }
		    if(strcmp(args[i][0], ">") == 0){
		        have_rediout = true;
		    }
		    if(strcmp(args[i][0], "<") == 0){
		        have_rediin = true;
		    }
		    i++;
		}
		int num_of_cmds = num_of_pipes + 1;
		//printf("%d\n", num_of_pipes);
		int fd[2 * num_of_cmds];
		i = 0;
		for (i = 0; i < num_of_cmds; i++) {
		    if (pipe(fd + i * 2) < 0) {
		        perror("couldn't pipe");
		        exit(EXIT_FAILURE);
		    }
		}
		int j = 0;
		for(int k = 0; k < com_num - 1; k++){
			if(strcmp(args[k][0], "|") != 0 && strcmp(args[k][0], ">") != 0 && strcmp(args[k][0], "<") != 0){
				pid = fork();
				//fputs(args[k][0], stdout);
				if(pid == -1){
				    perror("fork");
				    exit(1);
				}
				else if(pid == 0){
					if(have_pipe){
						if(k != com_num - 2){
							if (dup2(fd[j + 1], STDOUT_FILENO) < 0) {
							    perror("dup2");
							    exit(EXIT_FAILURE);
							}
						}
						if (k != 0) {
							if (dup2(fd[j - 2], STDIN_FILENO) < 0) {
							    perror("dup2");
							    exit(EXIT_FAILURE);
							}
						}
						
						for (i = 0; i < 2 * num_of_cmds; i++) {
							close(fd[i]);
						}
					}
					
					
					if(have_rediin == true && k == 0){
				    	int fi1 = open(args[1][1], O_RDONLY);
				    	if(fi1 < 0){
				    		perror("open");
				    		exit(1);
				    	}
				    	if(dup2(fi1, STDIN_FILENO) < 0) {
				    		perror("dup2");
						    exit(EXIT_FAILURE);
				    	}
				    	close(fi1);
				    }
				    if(have_rediout && have_rediin && !have_pipe){
							if(have_rediout == true && k == com_num - 4){
							int fi0 = open(args[k + 2][1], O_WRONLY|O_CREAT|O_TRUNC);
							if(fi0 < 0){
								perror("open");
								exit(1);
							}
							if(dup2(fi0, STDOUT_FILENO) < 0) {
								perror("dup2");
								exit(EXIT_FAILURE);
							}
							
							close(fi0);
						}
				    }
					else if(have_rediout == true && k == com_num - 3){
				    	int fi0 = open(args[k + 1][1], O_WRONLY|O_CREAT|O_TRUNC);
				    	if(fi0 < 0){
				    		perror("open");
				    		exit(1);
				    	}
				    	if(dup2(fi0, STDOUT_FILENO) < 0) {
				    		perror("dup2");
						    exit(EXIT_FAILURE);
				    	}
				    	
				    	close(fi0);
				    }
				    
				    if (execvp(args[k][0], args[k]) < 0) {
						perror(args[k][0]);
						exit(EXIT_FAILURE);
			    	}
				    
				}
				else {
				    waitpid(pid, NULL, 0);
				}
				j += 2;
		    }
		}
		
		for (i = 0; i < 2 * num_of_cmds; i++) {
		    close(fd[i]);
		}
	}
	else{
		printf("[Pid]: %d\n", back);
	}
	waitpid(pid, NULL, 0);
}

void pipe_f(char ***args){
	
    int i = 0;
    int num_of_pipes = 0;
    bool have_rediout = false, have_rediin = false, have_pipe = false;
    pid_t pid;
    
    while (args[i][0] != NULL){
        if(strcmp(args[i][0], "|") == 0){
            num_of_pipes++;
            have_pipe = true;
        }
        if(strcmp(args[i][0], ">") == 0){
            have_rediout = true;
        }
        if(strcmp(args[i][0], "<") == 0){
            have_rediin = true;
        }
        i++;
    }
    int num_of_cmds = num_of_pipes + 1;
    //printf("%d\n", num_of_pipes);
    int fd[2 * num_of_cmds];
    i = 0;
    for (i = 0; i < num_of_cmds; i++) {
        if (pipe(fd + i * 2) < 0) {
            perror("couldn't pipe");
            exit(EXIT_FAILURE);
        }
    }
    int j = 0;
    for(int k = 0; k < com_num; k++){
    	if(strcmp(args[k][0], "|") != 0 && strcmp(args[k][0], ">") != 0 && strcmp(args[k][0], "<") != 0){
		    pid = fork();
		    //fputs(args[k][0], stdout);
		    if(pid == -1){
		        perror("fork");
		        exit(1);
		    }
		    else if(pid == 0){
		   		//printf("a");
			    if(have_pipe){
			    	if(k != com_num - 1){
					    if (dup2(fd[j + 1], STDOUT_FILENO) < 0) {
					        perror("dup2");
					        exit(EXIT_FAILURE);
					    }
					}
					if (k != 0) {
					    if (dup2(fd[j - 2], STDIN_FILENO) < 0) {
					        perror("dup2");
					        exit(EXIT_FAILURE);
					    }
					}
					
					for (i = 0; i < 2 * num_of_cmds; i++) {
					    close(fd[i]);
					}
			    }
			    
			    
			    if(have_rediin == true && k == 0){
		        	int fi1 = open(args[1][1], O_RDONLY);
		        	if(fi1 < 0){
		        		perror("open");
		        		exit(1);
		        	}
		        	if(dup2(fi1, STDIN_FILENO) < 0) {
		        		perror("dup2");
				        exit(EXIT_FAILURE);
		        	}
		        	close(fi1);
		        }
		        if(have_rediout && have_rediin && !have_pipe){
				    	if(have_rediout == true && k == com_num - 3){
				    	int fi0 = open(args[k + 2][1], O_WRONLY|O_CREAT|O_TRUNC);
				    	if(fi0 < 0){
				    		perror("open");
				    		exit(1);
				    	}
				    	if(dup2(fi0, STDOUT_FILENO) < 0) {
				    		perror("dup2");
						    exit(EXIT_FAILURE);
				    	}
				    	
				    	close(fi0);
				    }
		        }
			    else if(have_rediout == true && k == com_num - 2){
		        	int fi0 = open(args[k + 1][1], O_WRONLY|O_CREAT|O_TRUNC);
		        	if(fi0 < 0){
		        		perror("open");
		        		exit(1);
		        	}
		        	if(dup2(fi0, STDOUT_FILENO) < 0) {
		        		perror("dup2");
				        exit(EXIT_FAILURE);
		        	}
		        	
		        	close(fi0);
		        }
		        
		        if(strcmp(args[k][0], "help\0") == 0){
					help_f();
					exit(0);
				}
				else if(strcmp(args[k][0], "cd\0") == 0){
					cd_f(args);
					exit(0);
				}
				else if(strcmp(args[k][0], "echo\0") == 0){
					echo_f(args);
					exit(0);
				}
				else if(strcmp(args[k][0], "exit\0") == 0){
					printf("See you next time\n");
					exit(0);
				}
				else if(strcmp(args[k][0], "record\0") == 0){
					printf("history cmd:\n");
					int nom;
					nom = 1;
					for(int j = 15; j >= 0; j--){
						if(strcmp(record[j], ",") != 0){
							printf("%d: ", nom);
							puts(record[j]);
							nom++;
						}
					}
					exit(0);
				}
				else if(strcmp(args[k][0], "replay\0") == 0){
					
					for(int i = 14; i >= 0; i--){
						strcpy(record[i + 1], record[i]);
						//fputs(record[i], stdout);
						//printf("\n");
					}
					
					strcpy(record[0], record[no - atoi(args[k][1])]);
					//printf("%d\n", no);
					char ***tmp = get_args(record[no - atoi(args[k][1])]);
					//puts(tmp[0][0]);
					commands_execute(tmp);
					exit(0);
				}
				else if(strcmp(args[k][0], "mypid\0") == 0){
					pid_t my, parent, child;
					if(strcmp(args[k][1], "-i") == 0){
						my = getpid();
						printf("%d\n", my);
					}
					else if(strcmp(args[k][1], "-p") == 0){
						char buffer[100];
						sprintf(buffer, "/proc/%d/stat", atoi(args[k][0]));
						FILE* fp = fopen(buffer, "r");
						if (fp) {
							size_t size = fread(buffer, sizeof (char), sizeof (buffer), fp);
							if (size > 0) {
								strtok(buffer, " "); // (1) pid  %d
								strtok(NULL, " "); // (2) comm  %s
								strtok(NULL, " "); // (3) state  %c
								char * s_ppid = strtok(NULL, " "); // (4) ppid  %d
								parent = atoi(s_ppid);
							}
							fclose(fp);
						}
						if(parent > 100000){
							printf("mypid -p :process id not exist.\n");
						}
						else{
							printf("%d\n", parent);
						}
					}
					else if(strcmp(args[k][1], "-c") == 0){
						char buffer[100];
						sprintf(buffer, "/proc/%d/task/%d/children", atoi(args[k + 1][0]), atoi(args[k + 1][0]));
						FILE* fp = fopen(buffer, "r");
						if (fp) {
							size_t size = fread(buffer, sizeof (char), sizeof (buffer), fp);
							//printf("%d\n", size);
							char *s_cpid = strtok(buffer, " ");
							while (s_cpid != NULL) {
								puts(s_cpid);
								s_cpid = strtok(NULL, " ");
							}
							fclose(fp);
						}
					}
					exit(0);
				}
		        else if (execvp(args[k][0], args[k]) < 0) {
					perror(args[k][0]);
					exit(EXIT_FAILURE);
	        	}
		        
		    }
		    else {
		        //printf("- fork %d\n", pid);
		        //waitpid(pid, NULL, 0);
		    }
		    j += 2;
        }
    }
    
    for (i = 0; i < 2 * num_of_cmds; i++) {
        close(fd[i]);
    }
    
    waitpid(pid, NULL, 0);
}

void cd_f(char ***args){
	if(args[1][0] == NULL){
		if (chdir(args[0][1]) != 0) {
		  perror("cd");
		}
	}
	else if (chdir(args[1][0]) != 0) {
      perror("cd");
    }
}

void echo_f(char ***args){
	if(strcmp(args[0][1], "-n") == 0){
		int i = 2;
		while (args[0][i] != NULL){
			if(args[0][i + 1] == NULL){
				printf("%s", args[0][i]);
			}
			else{
				printf("%s ", args[0][i]);
			}
        	i++;
    	}
	}
	else{
		int i = 1;
		while (args[0][i] != NULL){
			if(args[0][i + 1] == NULL){
				printf("%s\n", args[0][i]);
			}
			else{
				printf("%s ", args[0][i]);
			}
        	i++;
    	}
	}
	
}

int commands_execute(char ***args){
	
    pid_t pi;

    int i = 0;
    
    while (args[i][0] != NULL){
    	if(strcmp(args[i][0], "&") == 0){
    		background(args);
    		return 1;
    	}
        i++;
    }
    i = 0;
    while (args[i][0] != NULL){
    	if(strcmp(args[i][0], "|") == 0 || strcmp(args[i][0], ">") == 0 || strcmp(args[i][0], "<") == 0){
            pipe_f(args);
            return 1;
        }
        i++;
    }
	
	
    if(args[0][0] == NULL){
        return 1;
    }
    
    else if(strcmp(args[0][0], "help\0") == 0){
        help_f();
        return 1;
    }
    else if(strcmp(args[0][0], "cd\0") == 0){
        cd_f(args);
        return 1;
    }
    else if(strcmp(args[0][0], "echo\0") == 0){
        echo_f(args);
        return 1;
    }
    else if(strcmp(args[0][0], "exit\0") == 0){
    	printf("See you next time\n");
        exit(0);
        return 1;
    }
    else if(strcmp(args[0][0], "record\0") == 0){
    	printf("history cmd:\n");
    	int nom;
    	nom = 1;
        for(int j = 15; j >= 0; j--){
        	if(strcmp(record[j], ",") != 0){
		    	printf("%d: ", nom);
		    	puts(record[j]);
		    	nom++;
        	}
        }
        return 1;
    }
    else if(strcmp(args[0][0], "replay\0") == 0){
    	
		for(int i = 14; i >= 0; i--){
			strcpy(record[i + 1], record[i]);
			//fputs(record[i], stdout);
			//printf("\n");
		}
		
		strcpy(record[0], record[no - atoi(args[0][1])]);
    	//printf("%d\n", no);
    	char ***tmp = get_args(record[no - atoi(args[0][1])]);
    	//puts(tmp[0][0]);
    	return commands_execute(tmp);
    }
    else if(strcmp(args[0][0], "mypid\0") == 0){
    	pid_t my, parent, child;
    	if(strcmp(args[0][1], "-i") == 0){
	    	my = getpid();
	    	printf("%d\n", my);
    	}
    	else if(strcmp(args[0][1], "-p") == 0){
	    	char buffer[100];
			sprintf(buffer, "/proc/%d/stat", atoi(args[1][0]));
			FILE* fp = fopen(buffer, "r");
			if (fp) {
				size_t size = fread(buffer, sizeof (char), sizeof (buffer), fp);
				if (size > 0) {
					strtok(buffer, " "); // (1) pid  %d
					strtok(NULL, " "); // (2) comm  %s
					strtok(NULL, " "); // (3) state  %c
					char * s_ppid = strtok(NULL, " "); // (4) ppid  %d
					parent = atoi(s_ppid);
				}
				fclose(fp);
			}
			if(parent > 100000){
				printf("mypid -p :process id not exist.\n");
			}
			else{
				printf("%d\n", parent);
			}
		}
		else if(strcmp(args[0][1], "-c") == 0){
			char buffer[100];
			sprintf(buffer, "/proc/%d/task/%d/children", atoi(args[1][0]), atoi(args[1][0]));
			FILE* fp = fopen(buffer, "r");
			if (fp) {
				size_t size = fread(buffer, sizeof (char), sizeof (buffer), fp);
				
				char *s_cpid = strtok(buffer, " ");
				while (s_cpid != NULL) {
					puts(s_cpid);
					s_cpid = strtok(NULL, " ");
				}
				fclose(fp);
			}
		}
		return 1;
    }
    else{
        pi = fork();
        if(pi == -1){
            printf("Failed to fork\n");
        }
        else if(pi == 0){
            if(execvp(args[0][0], args[0]) == -1){
                perror("Error");
            }
        }
        else{
            int status;
            waitpid(pi, &status, 0);
            int err = WEXITSTATUS(status);
            if (err) { 
                perror("Error");
            }             
        }
        return 1;
    }
    
}

void loop(void){

	char *command;
	char ***args;
	int status;
	
	do{
		printf(">>> $ ");
		command = command_read();
		args = get_args(command);
		/*
        for(int i = 0; i < com_num; i++){
        	int j = 0;
            while(args[i][j] != NULL){
                fputs(args[i][j], stdout);
                printf(" ");
                j++;
            }
            printf("\n");
        }
        */
		status = commands_execute(args);
		
		free(command);
		free(args);
		
	}while(status);
}

int main(){
	
	//record = malloc(16 * sizeof(char*));
	for(int i = 0; i < 16; i++){
		//record[i] = (char*)malloc(100 * sizeof(char));
		strcpy(record[i], ",");
	}
	printf("=======================\n");
	printf("      Moshi moshi      \n");
	printf("  Welcome to my shell  \n");
	printf("=======================\n");
	
	loop();
	
	return 0;
}
