/*NAME: Sejal Vivek Kala
  Enroll :- BT20CSE057
  OPERATING SYSTEM
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

#define MAXSIZE 100
size_t bufferSize = 100;
void sigintHandler(int sig_num){
	signal(SIGINT, sigintHandler); 
}
void sighandler(int sig_num) {
    signal(SIGTSTP, sighandler); 
} 


void parsedInpcmd(char *inpCmd, char** commands){
	//'commands' is storing character arrays... i.e. array of character array
	// it stores separate command without includeing spaces in it
	char space[] = " ";
	char *cmd;
    
    inpCmd = strsep(&inpCmd, "\n");	//separating '\n' from string
	
    while(*inpCmd == ' ')	//removing starting white spaces...;
    {
        inpCmd++;
    }

    cmd = strdup(inpCmd); //duplicate cmd
    char *ptrToend = cmd + strlen(cmd)-1; 

    while(ptrToend > cmd && *ptrToend == ' ')	//removing end spaces...
    {
        ptrToend--; 
    }

    ptrToend[1] = '\0';

    char *token = strsep(&cmd, space);	//separating 1st command before space
    //Now 'cmd' string after 'token.
	if(strcmp(token, "cd") != 0)	//1st command is not "cd"
    {
		size_t it;
        for(it = 0; it < bufferSize; it++)
        {
            commands[it] = (char*)malloc(bufferSize*sizeof(char));
        }

        it = 0;
        while(token)
        {   
            token = strsep(&token, "\n"); //separating token string from '\n'
            commands[it] = token; //storing in commands
            token = strsep(&cmd, space); //getting next command
            it++;
        }
        commands[it] = NULL; //last  command as NULL
    }
    else  //cd is 1st 'token' i.e command
    {
        chdir(strsep(&cmd, "\n")); //command to change directory, strsep() returns ptr
    }
}

int parseInput(char* inpCmd)
{
	int ret = 0, j;
	char *ext = "exit\n";
	if(strcmp(inpCmd, ext)==0){
		return ret; //0
	}

	char *cmd = strdup(inpCmd);
	cmd = strsep(&inpCmd, "\n");
	int l = strlen(cmd);
	for(j=0; j<l-1; j++){
		
		if(cmd[j] == '&' && cmd[j+1] == '&'){
			ret = 1;
			return ret;
		}
		else if(cmd[j] == '#' && cmd[j+1] == '#'){
			ret = 2;
			return ret;
		}
		else if(cmd[j] == '>' ){
			ret = 3;
			return ret;
		}
	}	
	return 4; //for single command
}

void executeCommand(char* inpCmd)
{
	// This function will fork a new process to execute a command
	char **commands;
	commands = (char**)malloc(bufferSize*sizeof(char*));
	parsedInpcmd(inpCmd, commands);
	// separate_space(inpCmd, commands); 
	
	int rc = fork();
	if(rc<0)
	{
		exit(0);
	}
	else if (rc == 0)
	{
		signal(SIGTSTP, sighandler);  //enable signal again for child 
		signal(SIGINT, sigintHandler); 
	
		int ret = execvp(commands[0], commands);//returns -1 if error
        if(ret < 0) // error code execvp
        {
            printf("Shell: Incorrect command\n");
            exit(1);
        }
	}
	else 
	{
		wait(NULL);
	}	
}

int funcCmdCount(char *inpCmd, char* sep){
	int i=0, count=0;
	int l = strlen(inpCmd);
	for(int i=0; i<l; i++){
		if(inpCmd[i]=='\0')
			break;
		else if(inpCmd[i]==sep[0] && inpCmd[i+1]==sep[1]){
			count++;
		}
	}

	return count;
}

void executeAnd(char* commands){
	signal(SIGTSTP, sighandler); //Enable signal for child again
	signal(SIGINT, sigintHandler);

	char **cmd = (char**)malloc(bufferSize*sizeof(char*));
	parsedInpcmd(commands, cmd);

	if(cmd == NULL)
	{
		exit(0);
	}

	int ret = execvp(cmd[0], cmd);
	if(ret < 0)  //execvp error code
	{
		printf("Shell: Incorrect command\n");
		exit(1); 
	} 
}

void executeParallelCommands(char* inpCmd)
{
	// This function will run multiple commands in parallel 
	// multipleCmd = parseInput(inpCmd, commands, 1); //1 for parallel "&&"

	char sep[] = "&&"; //command separator
	int cmdCount = funcCmdCount(inpCmd, sep); //gives total count of "&&" in input
	char *sptr = strstr(inpCmd, sep); //first  "&&" that is ptr to "&&"
	char** commands = (char**)malloc(sizeof(char*)*(cmdCount+2)); //store string
	
	int i;
	for(i=0; i<=cmdCount; i++)
		commands[i] = (char*)malloc(MAXSIZE*sizeof(char)); //each string

	i=0;
	while(sptr)
    {
	
        sptr[0] = '\0';  //'&' is replaced by '\0'     
        char *str = strdup(inpCmd); //returns a pointer to a null-terminated string
        commands[i++] = str; //Each cmd is stored separately
        sptr[0] = ' ';  // '\0' changed to ' '
		// "&&" 2 characters are there...
        inpCmd = sptr+2; //pointer to next of "&&";
        sptr = strstr(inpCmd, sep); //pointer to next sep/"&&"
    }
	commands[i] = inpCmd;

	i=0;
	while(i<=cmdCount){
		int rc = fork();
		if(rc<0){
			exit(0); //exit if fork failed
		}
		else if(rc==0){
			executeAnd(commands[i]);
		}
		else{
			i++; //running parallel so no wait();
		}
	}
}

void executeSequentialCommands(char* inpCmd)
{		
	char sep[] = "##"; //command separator
	int cmdCount = funcCmdCount(inpCmd, sep); //gives total count of "##" in input
	char *sptr = strstr(inpCmd, sep); //first  "##" //ptr to sep

	char** commands = (char**)malloc(sizeof(char*)*(cmdCount+2)); //store string
	int i;
	for(i=0; i<=cmdCount; i++)
		commands[i] = (char*)malloc(MAXSIZE*sizeof(char)); //each string

	i=0;
	while(sptr)
    {
	
        sptr[0] = '\0';  //'&' is replaced by '\0'     
        char *str = strdup(inpCmd); //returns a pointer to a null-terminated string
        commands[i++] = str; //Each cmd is stored separately
        sptr[0] = ' ';  // '\0' changed to ' '
		// "&&" 2 characters are there...
        inpCmd = sptr + 2; //pointer to next of "##";
        sptr = strstr(inpCmd, sep);
    }
	commands[i] = inpCmd;

	for(int i=0; i<=cmdCount; i++){
        executeCommand(commands[i]);
	}
}

void executeCommandRedirection(char* inpCmd)
{
	// This function will run a single command with output redirected to an output file specificed by user
	char *exeCmd = strsep(&inpCmd, ">"); //'execmd' will store string before '>', thats our command
	char* fileName = inpCmd; //Now 'inpcmd' has string after '>' that is filename.
	while(*fileName == ' ') //extra whitespaces at start
	{
		fileName++;
	}
	int rc = fork();
	if(rc < 0){
		exit(0);	//fork fails
	}
	else if(rc == 0){
		close(STDOUT_FILENO);
		open(fileName, O_CREAT | O_RDWR, S_IRWXU);
		char **commands = (char**)malloc(bufferSize*sizeof(char*));;
		parsedInpcmd(exeCmd, commands);

		int ret = execvp(commands[0], commands);
        if (ret< 0) //execvp error code
		{ 
			printf("Shell: Incorrect command\n");
			exit(1);
		}
    }
    else
    {
        wait(NULL); 
    }
}

int main()
{
	// Initial declarations
	signal(SIGTSTP, sighandler); 
	signal(SIGINT, sigintHandler); 

	char currentWorkingDirectory[MAXSIZE];
	char *exitStr = "exit\n";
	char *inputStr;
	size_t ch;
	size_t inpsize = 32;
	inputStr = (char *)malloc(inpsize*sizeof(char));
	
	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));
		printf("%s$", currentWorkingDirectory);

		// accept input with 'getline()'
		ch = getline(&inputStr, &inpsize, stdin); 
		

		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		int ret = parseInput(inputStr); 		
		
		if( ret == 0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		else if(ret == 1) //"&&" is present
			executeParallelCommands(inputStr);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(ret == 2) //"##" is present
			executeSequentialCommands(inputStr);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(ret == 3) //">" is present 
			executeCommandRedirection(inputStr);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else
			executeCommand(inputStr);		// This function is invoked when user wants to run a single commands
	}
	return 0;
}