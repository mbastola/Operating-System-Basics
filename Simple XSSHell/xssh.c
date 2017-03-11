/*
MANIL BASTOLA
Simple XSSH shell
FILE: xssh.c
DEPENDENCIES: uthash.h, xssh.h
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "uthash.h"
#include "xssh.h"

int main(int argc, char** argv)
{
  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGINT, &sa, NULL) == -1)
    fprintf(stderr,"signal error\n");
  char* token;
  int fileflag = 0;
  const char* delim = "  \n";
  char input[BUFFERSIZE];
  FILE* inPATH = stdin;
  int option  = 0;
  char* debugflag;
  int pos = 0;
  int displayflag = 0;
  while ((option = getopt(argc, argv,"xd:f:"))!=-1)
    {
      switch(option)
        {
        case 'f':
          {
            fileflag = 1;
            filein = optarg;
            pos = pos+2;
            break;
          }
        case 'd':
          {
            debugflag = optarg;
            pos = pos+2;
            break;
          }
	case 'x':
	  {
	    displayflag = 1;
	    pos++;
	  }
        }
    }
  if (debugflag && !strcmp(debugflag, "1"))
    {
    printf("Debug Mode is on\n");
    }
  if (fileflag){
    inPATH = fopen(argv[pos],"r");
    if (argv[pos+1]){
      add("1", argv[pos+1]);
    } 
    if(argv[pos+2]){
      add("2", argv[pos+2]);
    }
    if (fileflag && inPATH == NULL) {
      fprintf(stderr,"Usage: ./xssh -f INFILE [$1] [$2]\n");
      exit(FAIL);
    }
  }
  else
    printf(">> ");
  while(fgets(input, BUFFERSIZE, inPATH) != NULL)
    {
      int i = 0;
      int j = 0;
      int file;
      int result;
      char* args[BUFFERSIZE];
      token = strtok(input, delim);
      while(token){
	if (!strcmp(token,"&")){
	  token = NULL;
	  backgroundflag = 1;
	}
	else if (!strcmp(token,"<")){
	  token = NULL;
	  stdIN = 0;
	  inPOS = i+1;
	}
	else if (!strcmp(token,">")){
	  token = NULL;
	  stdOUT = 0;
	  outPOS = i+1;
	  }
	if (i == inPOS)
	  {
	    file = open(token, O_RDONLY, 0);
	    dup2(file, STDIN_FILENO);
	    close(file);
	  }
	else if (i == outPOS)
	  {
	    //  needs work
	  }
	else{
	  if (token)
	    token = getVal(token);
	  args[j] = token;
	  j++;
	  if (displayflag)
	    printf("%s  ",args[i]);
	}
	i++;
	token = strtok(NULL, delim);
      }
      if (displayflag)
	printf("\n ");
      args[i] = NULL;
      if (args[0]){
	if(checkExternal(args[0])){
	  result = runExternal(args);
	}
	else{
	  result = runInternal(args);
	}
      }
      //close(file);
      if (result==FAIL)
	fprintf(stderr,"Command failed\n");
      if (!fileflag)
	printf(">> ");
    }
  if (fileflag)
    fclose(inPATH);
  return SUCCESS;
}

int checkExternal(char* args)
{
  int external = 1;
  if (!strcmp(args,"show") || !strcmp(args,"set") || !strcmp(args,"unset") || !strcmp(args,"export") || !strcmp(args,"unexport") || !strcmp(args,"chdir") || !strcmp(args,"exit") || !strcmp(args,"wait")){
    external = 0;
  }
  return external;
}

int runInternal(char** args)
{
  if(!strcmp(args[0],"show"))
    {
      int i = 1;
      while(args[i]){
	printf("%s ",args[i]);
	i++;
      }
      printf("\n");
      return SUCCESS;
    }
  else if (!strcmp(args[0],"set"))
    {
      if (args[1] && args[2]){
	add(args[1], args[2]);
      }
    }
  else if (!strcmp(args[0],"unset"))
    {
      int status;
      if (args[1]){
	status = removeKey(args[1]);
	if (status == FAIL) {
	  fprintf(stderr,"Usage: unset failed\n");
	  return FAIL;
	}
      }
      return SUCCESS;
    }
  else if (!strcmp(args[0],"export"))
    {
      int status;
      if (args[1] && args[2]){
	status = setenv(args[1],args[2],1);
	if (status == ERRNO) {
	  fprintf(stderr,"Usage: export failed\n");
	  return FAIL;
	}
	add(args[1], args[2]);
      }
      return SUCCESS;
    } 
  else if (!strcmp(args[0],"unexport"))
    {
      int status;
      if (args[1]){
	status = unsetenv(args[1]);
	if (status == ERRNO) {
	  fprintf(stderr,"Usage: unexport failed\n");
	  return FAIL;
	}
	removeKey(args[1]);
      }
      return SUCCESS;
    }
  else if (!strcmp(args[0],"chdir"))
    {
      int status;
      if (args[1]){
	status = chdir(args[1]);
	if (status == ERRNO) {
	  fprintf(stderr,"Usage: chdir failed\n");
	  return FAIL;
	}
	return status;
      }
    }
  else if (!strcmp(args[0],"exit"))
    {
      if (args[1])
	exit(atoi(args[1]));
      exit(0);
    }
  else if (!strcmp(args[0],"wait"))
    {
      int status=0;
      pid_t child = waitpid(args[1], &status, 0);
      return WEXITSTATUS(status);
    }
  return SUCCESS;
}

int runExternal(char** args)
{
  //char val[COMMANDSIZE];
  sprintf(command,"%d",getpid());
  add("$",command);
  int status = 0;
  pid_t pid = fork();
  if (pid == 0) //Child is running
    {
      execvp(args[0],args);
      fprintf(stderr,"Execvp not implemented by child.\n");
      return FAIL;
    }
  else if (pid > 0);
  {
    pid_t child;
    if (child == (pid_t)(-1)) {
      fprintf(stderr, "Fork failed.\n");
      return FAIL;
    }
    if (!backgroundflag){
      child = waitpid(child, &status,0);
      sprintf(command,"%d",WEXITSTATUS(status));
      add("?", command);
    }
    else{
      backgroundflag = 0; 
      sprintf(command,"%d", pid);
      add("!",command);
    }
  }
  //backgroundflag = 0;
  return SUCCESS;
} 

void handler(int sig)
{
  printf("\n");
  return;
}

char* getVal(char* variable)
{
  if (variable[0]='$'){
    strcpy(command,&variable[1]);
    struct enVar * ev = find(command);
    if (ev){
      return ev->value;
    }
    else
      {
	return NULL;
      }
  }
  else{
    return variable;
  }
}

