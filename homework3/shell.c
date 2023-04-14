//This is a simple shell by Haofan Wang

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>

//global variables
int fore = 0;
int kill_id[201];
int kill_counter = 0;

//function prototype
size_t readTokens(char*** toks, size_t* tok_num);

//signal handler for ^C
void handle_SIGINT(int signo) {
    char* message = "terminated by signal 2\n";
    write(STDOUT_FILENO, message, 23);
    fflush(stdout);
    _exit(2);
}

//signal handler for ^Z
void handle_SIGTSTP(int signo) {
    if (fore == 0) {  //check the foreground flag
        char* message = "\nEntering foreground-only mode (& is now ignored)\n:";
        write(STDOUT_FILENO, message, 51);
        fflush(stdout);
        fore = 1;  //set the foreground flag to ignore &
    }
    else {
        char* message = "\nExiting foreground-only mode\n:";
        write(STDOUT_FILENO, message, 31);
        fflush(stdout);
        fore = 0;  //set the flag to 0 to enable background mode
    }
}

int
main(int argc, char *argv[])
{
  char **toks = NULL;  //store tokens in it
  size_t toks_size;
  size_t num_toks;
  char *cwd = NULL;
  char *tmp_cwd;
  char *login;
  char hostname[HOST_NAME_MAX+1];
  int x = 0;
  int targetFD;  //target file
  int sourceFD;  //source file
  int result;
  int j;
  int z;
  pid_t childPid;  //store child pid
  int childStatus;  //check return status
  int f_childStatus;
  
  do
  {
      signal(SIGTSTP, SIG_DFL);  //don't ignore SIGTSTP
      signal(SIGINT, SIG_IGN);   //ignore SIGINT in parant process

      //take care of signal handlers
      //Initialize SIGTSTP_action struct to be empty
      struct sigaction SIGTSTP_action = { 0 };
      //Register handle_SIGTSTP as the signal handler
      SIGTSTP_action.sa_handler = handle_SIGTSTP;
      //Block all catchable signals while handle_SIGTSTP is running
      sigfillset(&SIGTSTP_action.sa_mask);
      //NO flags set
      SIGTSTP_action.sa_flags = SA_RESTART;

      //Install signal handler
      sigaction(SIGTSTP, &SIGTSTP_action, NULL);

      //This section is the lecture example code
    { /* Increase cwd buffer size until getcwd is successful */
      size_t len = 0;
      while (1) 
      {
        len += 16;
        cwd = realloc(cwd, len * sizeof *cwd);
        if (getcwd(cwd, len) == NULL)
        {
          if (errno == ERANGE) continue;
          err(errno, "getcwd failed");
        }
        else break;
      }
    }
    char *homedir = getenv("HOME");
    
    { /* Replace home directory prefix with ~ */
      size_t len = strlen(homedir);
      tmp_cwd = cwd;
      if(strncmp(tmp_cwd, homedir, len) == 0)
      {
        tmp_cwd += len-1;
        *tmp_cwd = '~';
      }
    }
    login = getlogin();
    gethostname(hostname, sizeof(hostname));

    //check and get the background child pid
    pid_t r_pid;
    kill_id[kill_counter] = r_pid;  //store pid into array (we will kill it later)
    kill_counter++;
    while ((r_pid = waitpid(-1, &childStatus, WNOHANG)) > 0) {
        if (WIFEXITED(childStatus)) {  //background process is finished
            printf("background pid %d is done: exit value %d\n", r_pid, WEXITSTATUS(childStatus));
            fflush(stdout);
        }
        else {  //background process is terminated
            printf("background pid %d is done: terminated by signal %d\n", r_pid, WTERMSIG(childStatus));
            fflush(stdout);
        }
 
    }
    /* Print out a simple prompt */
    printf(":");
    fflush(stdout);
    
    /* Call custom tokenizing function */
    num_toks = readTokens(&toks, &toks_size);
    size_t i;

    //if user didn't enter anything or just entered a command, then skip the following
    if (num_toks > 0)
    {
        //do the cd command, this section is from the example code
      if (strcmp(toks[0], "cd")==0)
      { /* cd command -- shell internals */
        if (num_toks == 1) 
        {
          if(chdir(homedir) == -1)
          {
            perror("Failed to change directory");
          }
        }
        else if (chdir(toks[1]) == -1)
        {
          perror("Failed to change directory");
        }
      }
      
      //implement status command
      else if (strcmp(toks[0], "status") == 0)
      { //check the return state of the last foreground child
          if (WIFEXITED(f_childStatus)) {
              printf("exit value %d\n", WEXITSTATUS(f_childStatus));
              fflush(stdout);
          }
          else {
              printf("terminated by signal %d\n", WTERMSIG(f_childStatus));
              fflush(stdout);
          }
      }
      
      //implementation of exit command, terminate all child process before exit
      else if (strcmp(toks[0], "exit") == 0)
      { 
          int d = 0;
          for (d; d < kill_counter; d++) {
              killpg(kill_id[d], SIGTERM);
          }
          exit(0);
      }
      else
      { /* Default behavior: fork and exec */
          //check whether it is background or foreground
          if (strcmp(toks[num_toks - 1], "&") != 0 || fore == 1) {  //foreground
              
              //get rid of the & postfix
              if (strcmp(toks[num_toks - 1], "&") == 0) {
                  toks[num_toks - 1] = NULL;
                  num_toks--;
              }
              
              pid_t pid = fork();
              
              if (pid == 0)
              { /* child */
                  //foreground child process ignores SIGTSTP and accepts SIGINT
                  signal(SIGTSTP, SIG_IGN);
                  signal(SIGINT, SIG_DFL);
                  //take care of signal handlers
                  
              //Initialize SIGINT_action struct to be empty
                  struct sigaction SIGINT_action = { 0 };
                  //Register handle_SIGIINT as the signal handler
                  SIGINT_action.sa_handler = handle_SIGINT;
                  //Block all catchable signals while handle_SIGINT is running
                  sigfillset(&SIGINT_action.sa_mask);
                  //NO flags set
                  SIGINT_action.sa_flags = SA_RESTART;

                  //Install signal handler
                  sigaction(SIGINT, &SIGINT_action, NULL);

                  //take care of redirect
                  for (x = 0; x < num_toks; x++) {
                      //input redirection
                      if (strcmp(toks[x], "<") == 0) {
                          //open source file
                          sourceFD = open(toks[x + 1], O_RDONLY);
                          if (sourceFD == -1) {
                              char* message;
                              sprintf(message, "cannot open %s for input\n", toks[x + 1]);
                              printf("%s", message);
                              fflush(stdout);
                              exit(1);
                          }
                          //redirection
                          result = dup2(sourceFD, 0);
                          if (result == -1) {
                              perror("source dup2()");
                              exit(2);
                          }
                          //now rearrange toks
                          num_toks = num_toks - 2;
                          for (j = x; j < num_toks; j++) {
                              toks[j] = toks[j + 2];
                          }
                          toks[num_toks] = NULL;
                          toks[num_toks + 1] = NULL;
                          x--;
                      }
                      //output redirection
                      if (strcmp(toks[x], ">") == 0) {
                          targetFD = open(toks[x + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                          if (targetFD == -1) {
                              perror("open()");
                              exit(1);
                          }

                          result = dup2(targetFD, 1);
                          if (result == -1) {
                              perror("dup2");
                              exit(2);
                          }
                          //now rearrange toks
                          num_toks = num_toks - 2;
                          for (j = x; j < num_toks; j++) {
                              toks[j] = toks[j + 2];
                          }
                          toks[num_toks] = NULL;
                          toks[num_toks + 1] = NULL;
                          x--;  //toks[x] has been changed, so check it again
                      }
                  }
                  
                  execvp(toks[0], toks);
                  errx(1, "%s: no such file or directory", toks[0]);  //customised error message
              }

              //if the child is terminated by a signal, then the parent will report the termination.
              childPid = waitpid(pid, &f_childStatus, 0);
              kill_id[kill_counter] = childPid;  //push pid to the array
              kill_counter++;
              if (WIFSIGNALED(f_childStatus)) {
                  printf("terminated by signal %d\n", WTERMSIG(f_childStatus));
                  fflush(stdout);
              }
          }
          else if(strcmp(toks[num_toks - 1], "&") == 0){  //background
            //get rid of &
              toks[num_toks - 1] = NULL;
              num_toks--;

              int bpid;
              pid_t pid = fork();
              bpid = getpid();

              //g_pid = &pid;
              if (pid == 0)
              { /* child */
                  //background child process ignore SIGTSTP and SIGINT
                  signal(SIGTSTP, SIG_IGN);
                  signal(SIGINT, SIG_IGN);
                  printf("background pid is %d\n", bpid);
                  fflush(stdout);

                  //take care of redirect
                  int inputFlag = 0;
                  int outputFlag = 0;
                  for (x = 0; x < num_toks; x++) {
                      //input redirection
                      if (strcmp(toks[x], "<") == 0) {
                          //raise the input flag
                          inputFlag = 1;
                          //open source file
                          sourceFD = open(toks[x + 1], O_RDONLY);
                          if (sourceFD == -1) {
                              perror("source open()");
                              exit(1);
                          }
                          //redirection
                          result = dup2(sourceFD, 0);
                          if (result == -1) {
                              perror("source dup2()");
                              exit(2);
                          }
                          //now rearrange toks
                          num_toks = num_toks - 2;
                          for (j = x; j < num_toks; j++) {
                              toks[j] = toks[j + 2];
                          }
                          toks[num_toks] = NULL;
                          toks[num_toks + 1] = NULL;
                          x--;
                      }
                      //output redirection
                      if (strcmp(toks[x], ">") == 0) {
                          //raise output flag
                          outputFlag = 1;
                          targetFD = open(toks[x + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                          if (targetFD == -1) {
                              perror("open()");
                              exit(1);
                          }

                          result = dup2(targetFD, 1);
                          if (result == -1) {
                              perror("dup2");
                              exit(2);
                          }
                          //now rearrange toks
                          num_toks = num_toks - 2;
                          for (j = x; j < num_toks; j++) {
                              toks[j] = toks[j + 2];
                          }
                          toks[num_toks] = NULL;
                          toks[num_toks + 1] = NULL;
                          x--;
                      }
                  }
                  //if user didn't include source or target file, redirect to /dev/null
                  if (inputFlag == 0) {
                      //redirect to /dev/null
                      int devNull = open("/dev/null", O_RDONLY);
                      result = dup2(devNull, 0);
                      if (result == -1) {
                          perror("source dup2()");
                          exit(2);
                      }
                  }
                  if (outputFlag == 0) {
                      //redirect to /dev/null
                      int devNull = open("/dev/null", O_WRONLY);
                      result = dup2(devNull, 1);
                      if (result == -1) {
                          perror("dup2");
                          exit(2);
                      }
                  }

                  execvp(toks[0], toks);
                  err(errno, "failed to exec");
              }
          }
        
      }
    }
    //initialize the toks array
    for (z = 0; z < 520; ++z) {
        toks[z] = NULL;
    }
  } while (num_toks >= 0);
  
  //free variables
  size_t i;
  for (i=0; i<toks_size; ++i)
  {
    free(toks[i]);
  }
  free(toks);
  free(cwd);

}

//this function read the command and break them into tokens for the shell to execute
size_t
readTokens(char*** toks, size_t* tok_num) {
    //declare the toks array and initialize
    *tok_num = 520;  //hardcode the max amount of token
    if (*toks == NULL) {
        *tok_num = 520;
        if ((*toks = malloc(520 * sizeof(char*))) == NULL)
        {
            err(errno, "malloc failed");
        }
        size_t i;
        //initialize the toks array to NULL
        for (i = 0; i < 520; ++i) {
            (*toks)[i] = NULL;
        }
    }

    //parse commands from the shell
    char* command = NULL;
    char* saveptr;
    size_t len = 0;
    ssize_t nread;
    int counter = 0; //this will be the number of valid tokens in toks

    nread = getline(&command, &len, stdin);  //now the line is stored in command
    

    //if the command is a blank line, return 0 because there's 0 valid token
    if (nread == 1) {
        return 0;
    }
    //if the command start with #, return 0 becasue there's 0 valid token
    else if (strncmp(command, "#", 1) == 0) {
        return 0;
    }

    //get tokens surrounded by spsce or nextline, and store them in toks
    else {
        //if the command contains $$, replace it with pid.
        //get the pid
        int dollar_id = getpid();
        
        //convert dollar_id from int to char
        char char_id[10];
        sprintf(char_id, "%d", dollar_id);
        char* dollar = "$$";
        char* mark;
        
        //search for $$ pattern in command
        char buffer[3000];
        strcpy(buffer, command);
        char command1[3000];
        int changeFlag = 0;
        //strstr will search for pattern in buffer, and mark will point to the first pattern
        while (mark = strstr(buffer, dollar)) {
            *mark = '\0';   //change it to terminate character, that buffer now is just the first chunk of command
            char* first;
            first = buffer;
            //reform the actrual command
            sprintf(command1, "%s%s%s", first, char_id, mark + 2);
            strcpy(buffer, command1);
            changeFlag = 1;
        }
        //if we changed the command, then the actrual command is stored in command1, we need to get it back
        if (changeFlag == 1) {
            command = realloc(command, sizeof(command1));
            strcpy(command, command1);
        }
        
        //parse the command into different tokens
        char* element = strtok_r(command, " \n", &saveptr);

        while (element != NULL) {
            (*toks)[counter] = calloc(strlen(element) + 1, sizeof(char));
            strcpy((*toks)[counter], element);

            
            counter++;
            element = strtok_r(NULL, " \n", &saveptr);
        }
    }

    free(command);
    return counter;
}