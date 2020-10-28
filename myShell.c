#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <sys/wait.h> 

char SHELL_NAME[50] = "fsh>";
int QUIT = 0;

// Function to read a line from command into the buffer
char *readLine()
{
	char *line = (char *)malloc(sizeof(char) * 1024); // Dynamically Allocate Buffer
	char a;
	int pos = 0, buf = 1024;
	if (!line) // Buffer Allocation Failed
	{
		printf("\nBuffer Allocation Error.");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		a=getchar();
		if (a == EOF || a == '\n') // If End of File or New line, replace with Null character
		{
			line[pos] = '\0';
			return line;
		}
		else
		{
			line[p] = a;
		}
		p ++;
		// If we have exceeded the buffer
		if (p >= buf)
		{
			buf += 1024;
			line = realloc(line, sizeof(char) * buf);
			if (!line) // Buffer Allocation Failed
			{
			printf("\nBuffer Allocation Error.");
			exit(EXIT_FAILURE);
			}
		}
	}

}

// Function to split a line into constituent commands
char **splitLine(char *line)
{
	char **tokens = (char **)malloc(sizeof(char *) * 64);
	char *token;
	char delim[10] = " \t\n\r\a";
	int p = 0, buf = 64;
	if (!tokens)
	{
		printf("\nBuffer Allocation Error.");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, delim);
	while (token != NULL)
	{
		tokens[p] = token;
		p ++;
		if (p >= buf)
		{
			buf += 64;
			line = realloc(line, buf * sizeof(char *));
			if (!line) // Buffer Allocation Failed
			{
			printf("\nBuffer Allocation Error.");
			exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, delim);
	}
	tokens[p] = NULL;
	return tokens;
}

// Section Dealing with Built-in Commands

// Function Declarations
int myShell_cd(char **args);
int myShell_exit();

// Definitions
char *builtin_cmd[] = {"cd", "exit"};

int (*builtin_func[]) (char **) = {&myShell_cd, &myShell_exit}; // Array of function pointers for call from execShell

int numBuiltin() // Function to return number of builtin commands
{
	return sizeof(builtin_cmd)/sizeof(char *);
}

// Builtin command definitions
int myShell_cd(char **args)
{
	if (args[1] == NULL) 
	{
		printf("myShell: expected argument to \"cd\"\n");
	} 
	else 
	{
		if (chdir(args[1]) != 0) 
		{
			perror("myShell: ");
		}
	}
	return 1;
}

int myShell_exit()
{
	QUIT = 1;
	return 0;
}

// Function to create child process and run command
int myShellLaunch(char **args)
{
	pid_t pid, wpid;
	int status;
	pid = fork();
	if (pid == 0)
	{
		// The Child Process
		if (execvp(args[0], args) == -1)
		{
			perror("myShell: ");
		}
	exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		//Forking Error
		perror("myShell: ");
	}
	else
	{
		// The Parent Process
	do 
	{
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

// Function to execute command from terminal
int execShell(char **args)
{
	int ret;
	if (args[0] == NULL)
	{
		// Empty command
		return 1;
	}
	// Loop to check for builtin functions
	for (int i=0; i< numBuiltin(); i++) // numBuiltin() returns the number of builtin functions
	{
		if(strcmp(args[0], builtin_cmd[i])==0) // Check if user function matches builtin function name
			return (*builtin_func[i])(args); // Call respective builtin function with arguments
	}
	ret = myShellLaunch(args);
	return ret;
}

// Read and Parse from Config File
int readConfig()
{
	FILE *fptr;
	char line[200];
	char **args;
	fptr = fopen("config", "r");
	if (fptr == NULL)
	{
		printf("Unable to find config file.\n");
		return 1;
	}
	else
	{
		while(fgets(line, sizeof(line), fptr) != NULL)
		{
			printf("\n%s", line);
			args=splitLine(line);
			if(strcmp(args[0], "export")==0)
				strcpy(SHELL_NAME, args[1]);
		}
	}
	free(args);
	fclose(fptr);
	return 1;
}

// When myShell is called Interactively
int myShellInteract()
{
	char *line;
	char **args;
	while(QUIT == 0)
	{
		printf(SHELL_NAME);
		line=readLine();
		args=splitLine(line);
		execShell(args);
		free(line);
		free(args);
	}
	return 1;
}

// When myShell is called with a Script as Argument
int myShellScript(char filename[100])
{
	printf("Received Script. Opening %s", filename);
	FILE *fptr;
	char line[200];
	char **args;
	fptr = fopen(filename, "r");
	if (fptr == NULL)
	{
		printf("\nUnable to open file.");
		return 1;
	}
	else
	{
		printf("\nFile Opened. Parsing. Parsed commands displayed first.");
		while(fgets(line, sizeof(line), fptr)!= NULL)
		{
			printf("\n%s", line);
			args=splitLine(line);
			execShell(args);
		}
	}
	free(args);
	fclose(fptr);
	return 1;
}

int main(int argc, char **argv)
{
	// Read from myShell Configuration Files
	readConfig();
	// Parsing commands Interactive mode or Script Mode
	if (argc == 1)
		myShellInteract();
	else if (argc == 2)
		myShellScript(argv[1]);
	else
		printf("\nInvalid Number of Arguments");

	// Exit the Shell
	return EXIT_SUCCESS;
}
