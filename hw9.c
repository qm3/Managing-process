#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <readline/readline.h>
#include <readline/history.h>

//---------------------------------------------------
// This code implements one functionality required by
// your shell -- it reads a command line, produces an
// array of char * pointers to the tokens on the command
// line, and frees storage.
//---------------------------------------------------

#define PROMPT "hw9> "

#define MAX_TOKENS 16

#define DIRECTION_NONE 0
#define DIRECTION_STDIN 1
#define DIRECTION_STDOUT 2

#define BACKGROUND_NO 0
#define BACKGROUND_YES 1

#define STD_ACCESS  (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef enum {IN_WS, IN_TOKEN} State;
static void processCommandLine(char ** token, int tokenNum);
static int openFile(char * fileName, int fileAccess);
//static int closeFile(int Fdesc);

int main(int argc, char *argv[]) {

  char *line = NULL;          // line of input from stdin
  char *token[MAX_TOKENS+1];  // array of pointers to tokens
                              // must have a NULL to end list of pointers
  
  while (1) {
    //----------------------------------------
    // free memory allocated last time around
    //---------------------------------------
    if ( line ) free(line);

    //-----------------------------------------
    // Get a line of input
    //-----------------------------------------
    line = readline(PROMPT);
    if ( line == NULL ) break;      // EOF
    if ( *line == '\0' ) continue;  // Empty line
    add_history(line);

    //-------------------------------
    // break line into tokens
    //-------------------------------
    State state = IN_WS;
    int tokenNum = 0;
    for (char *pChar=line; *pChar; pChar++) {   // if pChar is NULL, break.
      if ( isspace(*pChar) ) {
        if ( state == IN_TOKEN ) {
          *pChar = '\0';
          tokenNum++;
          state = IN_WS;
	      }
      }
      else {
        if ( state == IN_WS && tokenNum < MAX_TOKENS ) {
          token[tokenNum] = pChar;
          state = IN_TOKEN;
        }
      }
    }

    if ( state == IN_TOKEN ) {
      tokenNum++;
    }
    
    token[tokenNum] = NULL;

    //-------------------------------------------------
    // Now print the tokens
    //-------------------------------------------------
    
    printf("\nTokens:\n");
    for (int i=0; token[i] != NULL; i++) {
      printf("[%d] '%s'\n", i, token[i]);
    }
    
    
    processCommandLine(token, tokenNum);
  }
  // free last line used, if there is one
  if ( line ) free(line);


  return EXIT_SUCCESS;
}

static void processCommandLine(char ** token, int numTokens) {
  char * prog1ParmList[MAX_TOKENS];
  char chr;
  int tokenIndex;
  int numCommands;  // number of commands we are passing into the exec
  int tokenLen;   // 
  int direction;
  int runBackground;
  int pid;  // process ID
  int Fdesc;  // file descriptor

  // validate the request
  if (numTokens == 0) {
    printf("Error: no parameters specified.\n");
    return;
  }

  // start building the parameter list for the target executable command.
  direction = DIRECTION_NONE;

  for (tokenIndex = 0; (tokenIndex < numTokens); tokenIndex++) {
    tokenLen = strlen(token[tokenIndex]);
    chr = token[tokenIndex][0];

    if ((chr == ('<')) && (tokenLen == 1)) {
      direction = DIRECTION_STDIN;
      break;
    }

    if ((chr == ('>')) && (tokenLen == 1)) {
      direction = DIRECTION_STDOUT;
      break;
    }

    if ((chr == ('&')) && (tokenLen == 1)) {
      break;
    }

    prog1ParmList[tokenIndex] = malloc(tokenLen + 1);
    strncpy(prog1ParmList[tokenIndex], token[tokenIndex], tokenLen);

  }

  // Terminate the end of the parameter list.
  prog1ParmList[tokenIndex] = NULL;
  numCommands = tokenIndex;

  runBackground = BACKGROUND_NO;
  chr = token[numTokens - 1][0];
  if ((chr == ('&')) && (tokenLen == 1)) {
    runBackground = BACKGROUND_YES;
  }
  // printf("RunBackground: [%d]\n", runBackground);


  // We are now ready to create the child process for the new command.
  pid = fork();

  if (pid < 0) {  // if fork failed
    printf("Error: fork failed.\n");
    return;
  }

  if (pid == 0) { // if this is the child process

    // if we are redirecting standard input
    if (direction == DIRECTION_STDIN) {

      if ((numCommands + 1) >= numTokens) {
        printf("Error: Missing input file.\n");
        return;
      }

      // close standard input.
      close(STDIN_FILENO);
      // open the file that the user specified for input
      Fdesc = openFile(token[numCommands + 1], O_RDWR);

      if (Fdesc < 0) {
        printf("Error: Unable to open input file.\n");
        return;
      }      
      // the following call will replace stdin with the opened file descriptor
      // the lowest unused device will be replaced
      dup(Fdesc);

    // if we are redirecting standard output
    } else if (direction == DIRECTION_STDOUT) {
      
      if ((numCommands + 1) >= numTokens) {
        printf("Error: Missing output file.\n");
        return;
      }

      // close standard output.
      close(STDOUT_FILENO);
      // open the file that the user specified for output
      Fdesc = openFile(token[numCommands + 1], (O_CREAT | O_WRONLY));

      if (Fdesc < 0) {
        printf("Error: Unable to open input file.\n");
        return;
      }
      
      // the following call will replace stdout with the opened file descriptor
      // the lowest unused device will be replaced
      dup(Fdesc);

    } 


    // launch the program.
    execvp(prog1ParmList[0], prog1ParmList);
    printf("Error: execvp failed.\n");
    return;

  }

  // Free the command line memory. 
  for (tokenIndex = 0; (tokenIndex < numCommands); tokenIndex++) {
    free(prog1ParmList[tokenIndex]);
  }

  // Wait the child to finish. But if running in the background, 
  // don't wait for it.
  if (runBackground == BACKGROUND_NO) {
    waitpid(pid, NULL, 0);
  }

}

static int openFile(char * fileName, int fileAccess) {

  int Fdesc;

  if ((fileAccess & O_CREAT) == 0) {
    Fdesc = open(fileName, fileAccess);
  } else {
    Fdesc = open(fileName, fileAccess, STD_ACCESS);
  }

  return (Fdesc);

}

/*
static int closeFile(int Fdesc) {

  int err;

  err = close(Fdesc);

  return (err);

}
*/