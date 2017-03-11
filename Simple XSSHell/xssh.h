/*
MANIL BASTOLA
CSE 422 LAB 1
FILE: xssh.h
DEPENDENCIES: uthash.h
 */

#ifndef _XSSH_H
#define _XSSH_H


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "uthash.h"

#define BUFFERSIZE 1024
#define COMMANDSIZE 256
#define SUCCESS 0
#define FAIL 1
#define ERRNO -1
int runExternal(char** input);
int checkExternal(char* input);
int runInternal(char** input);
char* filein;
int backgroundflag = 0;
int stdIN = 1;
int stdOUT = 1;
int inPOS = -1;
int outPOS = -1;
void handler(int signum);
char* getVal(char* variable);
char command[COMMANDSIZE];

/*Hash table borrowed from UTHASH website of http://troydhanson.github.io/uthash/*/
struct enVar {
  char key[COMMANDSIZE];
  char value[COMMANDSIZE];
  UT_hash_handle hh;
};

//Environment variables hash table
struct enVar * envHashTable = NULL;

//finds the struct given a key return NULL otherwise
struct enVar * find(char* key) {
  struct enVar * var;
  HASH_FIND_STR(envHashTable, key, var);
  return var;
}

//adds a struct key pair to the environemnt hash table.
void add(char key[COMMANDSIZE], char value[COMMANDSIZE]) {
  struct enVar * var = find(key);
  if (var == NULL){
    var = malloc(sizeof(struct enVar));
    strcpy(var->key, key);
  }
  strcpy(var->value, value);
  HASH_ADD_STR(envHashTable, key, var);
}

//deletes the struct key pair
int removeKey(char key[COMMANDSIZE]) {
  struct enVar * var = find(key);
  if (var == NULL)
    {
      return FAIL;
    }
  HASH_DEL(envHashTable, var);
  free(var);
  return SUCCESS;
}


#endif /* _XSSH_H */
