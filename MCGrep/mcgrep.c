/*
MANIL BASTOLA
FILE: mcgrep.c
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define BUFFERSIZE 100
#define SUCCESS 0
#define FAIL 1
#define COMMANDSIZE 10
#define FILE_MODE (S_IWRITE | S_IREAD) 
size_t fileSize;
char* findChar;      /*find this character*/
char* replaceWith;   /*replace it with this character*/
int replaceFlag;     /*keeps record of -r option*/ 

/*Takes input arguments as input filename and number of threads. Opens the input file from the filename. Memcopies the entire file.
Creates "numThread" number of output files. Breaks the input files into "numthread" number of chunks. Writes each chunk to an output file.
After writing, unmaps the input and output files form memory. Finally, closes all the streams*/
void parse(char* filename, int numThread);

/*If -r option is not raisedm this function searches through the specified text file and find all instances
of a single character and print each line that contains the specified character. If replace flag is raised,
the replacement character replaces any instance of the search character
that is found.  */
void* grep(int input);

/*Takes number of threads. Opens the output file stream. Memcopies the chunk sized tmp .out files.
 Combines the tmp files into stdout.After writing, unmaps the input and output streams form memory. Finally, closes all the streams*/
void wrapToFile(char* filename, int numThread);

/*Same as above. Only prints out to stdout*/
void wrapToStdout(int numThread);

int main(int argc, char** argv)
{
  struct timespec tstart={0,0}, tend={0,0};
  clock_gettime(CLOCK_MONOTONIC, &tstart);
  int option  = 0;
  int pos = 0;
  char fileInName[COMMANDSIZE];
  int numThreads = 1;
  replaceFlag = 0;
  int outfileFlag=0;
  char fileOutName[COMMANDSIZE];
  while ((option = getopt(argc, argv,"i:c:r:t:o:"))!=-1)
    {
      switch(option)
        {
	case 'i':
          {
	    strcpy(fileInName,optarg);
	    break;
          }
	case 'c':
          {
	    findChar = optarg;
            break;
          }
        case 'r':
          {
            replaceWith = optarg;
	    /*if find and replace char are different*/
	    if (*replaceWith != *findChar) 
	      replaceFlag = 1;
            break;
          }
	case 't':
	  {
	    numThreads = atoi(optarg);
	    break;
	  }
	  /*This is an optional parameter. Helpful to diff out files*/
	case 'o':
	  {
	    outfileFlag = 1;
	    strcpy(fileOutName,optarg);
	  }
        }
    }
  if ( !strlen(fileInName) || findChar == NULL || (replaceFlag && replaceWith == NULL) || numThreads < 1 ) 
    {
      fprintf(stderr,"Usage: ./mcgrep -i <input_file> -c <search char> [-r <replacement char>] [-t <# of threads>]\n");
      exit(FAIL);
    }
  parse(fileInName, numThreads);
  int i;
  pthread_t table[numThreads];
  for(i = 1; i <= numThreads; ++i)
    {
      /*for sequential running of grep*/
      //grep(i);
      /*for parallel running for grep*/
      pthread_t task;
      pthread_create(&task, NULL, grep, i);
      table[i-1] = task;      
    }
  for(i = 1; i <= numThreads; ++i)
    {
      if(pthread_join(table[i-1], NULL))
	{
	  fprintf(stderr, "Error joining one of the threads\n");
	  exit(FAIL);
	}
    }
  if (outfileFlag){
    printf("Output printed to --> %s file\n",fileOutName);
    wrapToFile(fileOutName, numThreads);
  }
  else{
    wrapToStdout(numThreads);
  }
  clock_gettime(CLOCK_MONOTONIC, &tend);
  printf("Mcgrep execution took ~ %.5f seconds\n",
	 ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
	 ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
  return SUCCESS;
}

void parse(char* filename, int numThread)
{
  int fileDes, fileDesOut ;
  struct stat s;
  int status;
  char *dst, *src;
  /*input file descriptor. option read*/
  fileDes = open(filename, O_RDONLY);
  if (fileDes < 0 ){
    fprintf(stderr, "Can't open one of the files\n");
  }
  /*peek size of the file*/
  status = fstat(fileDes, &s);
  fileSize = s.st_size;
  if (status < 0){
    fprintf(stderr, "Failed to load stat for %s\n", filename);
  }
  /*map the input file into memory*/
  src = mmap(0, fileSize, PROT_READ, MAP_SHARED, fileDes, 0);
  if (src == MAP_FAILED){
    fprintf(stderr,"mmapping for %s failed\n",filename);
  }
  int chunk = fileSize/numThread;
  int i = 0;
  int k = 1;
  while (i < fileSize){
    char str[10];
    snprintf(str, 10, "%d.tmp", k);
    const char* fileOutname = str;
    /*output file descriptor. access read, write. permissions rw*/
    fileDesOut = open(fileOutname, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
    if (fileDes < 0){
      fprintf(stderr, "Can't open one of the files\n");
    }
    if (lseek (fileDesOut, chunk - 1, SEEK_SET) == -1)
      fprintf(stderr,"lseek error");
    /* write a dummy byte at the last location */
    if (write (fileDesOut, "", 1) != 1)
      fprintf(stderr,"write error");
    /*map the output file into memory*/
    dst = mmap(0, chunk, PROT_READ | PROT_WRITE, MAP_SHARED, fileDesOut, 0);
    if (dst == MAP_FAILED){
      fprintf(stderr,"mmapping for %s failed\n",fileOutname);
    }
    /*copy memory contents from sourcec to destination*/
    memcpy (dst, src+i, chunk);
    /*unmap*/
    if (munmap(dst, chunk) == -1) 
      {
	fprintf(stderr,"munmap for %s failed\n",fileOutname);
      }
    /*close opened files*/
    close(fileDesOut);
    i+=chunk;
    ++k;
  }
  /*unmap & close*/
  if (munmap(src, fileSize) == -1) 
    {
      fprintf(stderr,"munmap failed with error:");
    }
  close(fileDes);
  return;
}

void* grep(int filenum)
{
  char str[10];
  char str2[10];
  snprintf(str, 10, "%d.tmp", filenum);
  snprintf(str2, 10, "%d.out", filenum);
  const char* filename = str;
  const char* filename2 = str2;
  FILE *fp = fopen(filename,"r");
  FILE *fp2 = fopen(filename2,"w");
  char line[BUFFERSIZE];
  int found;
  while(fgets(line, sizeof line, fp))
    {
      found = 0;
      int length = strlen(line);
      int i;
      for(i=0;i<length;++i)
        {
	  /*if replace option is not used, just print the line out*/
	  if (line[i] == *findChar && !replaceFlag)
	    {
	      found = 1;
	      break;
	    }
	  /*else repace characters*/
	  else if (line[i] == *findChar && replaceFlag)
	    {
	      found = 1;
	      line[i] = *replaceWith;
	    }
        }
      if (found)
	fprintf(fp2, line);
    }
  fclose(fp);
  fclose(fp2);
}

void wrapToFile(char* filename, int numThread)
{
  int fileDes, fileDesOut ;
  struct stat s;
  int status;
  int chunkSize;
  char *dst, *src;
  fileDesOut = open(filename, O_RDWR | O_CREAT | O_TRUNC, FILE_MODE);
  if (fileDesOut < 0 ){
    fprintf(stderr, "Can't open %s\n", filename);
  }
  dst = mmap(0, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDesOut, 0);
  if (dst == MAP_FAILED){
    fprintf(stderr,"mmapping for %s failed\n",filename);
  }
  if (lseek (fileDesOut, fileSize - 1, SEEK_SET) == -1)
    fprintf(stderr,"lseek error");
  if (write (fileDesOut, "", 1) != 1)
    fprintf(stderr,"write error");
  int i = 0;
  int k = 1;
  while (k <= numThread){
    char str[10];
    snprintf(str, 10, "%d.out", k);
    const char* fileOutname = str;
    fileDes = open(fileOutname, O_RDONLY);
    status = fstat(fileDes, &s);
    chunkSize = s.st_size;
    if (status < 0){
      fprintf(stderr, "Failed to load stat for %s\n", fileOutname);
    }
    if (fileDes < 0){
      fprintf(stderr, "Can't open %s\n",fileOutname);
    }
    src = mmap(0, chunkSize, PROT_READ, MAP_SHARED, fileDes, 0);;
    if (src == MAP_FAILED){
      fprintf(stderr,"mmapping for %s failed\n",fileOutname);
    }
    memcpy (dst+i, src, chunkSize);
    if (munmap(src, chunkSize) == -1) 
      {
	fprintf(stderr,"munmap for %s failed\n",fileOutname);
      }
    close(fileDes);
    i+=chunkSize;
    ++k;
  }
  close(fileDesOut);
  return;
}


void wrapToStdout(int numThread)
{
  int fileDes;
  struct stat s;
  int status;
  int chunkSize;
  char *src;
  int i = 0;
  int k = 1;
  char* buffer;
  while (k <= numThread){
    char str[10];
    snprintf(str, 10, "%d.out", k);
    const char* fileOutname = str;
    fileDes = open(fileOutname, O_RDONLY);
    status = fstat(fileDes, &s);
    chunkSize = s.st_size;
    if (status < 0){
      fprintf(stderr, "Failed to load stat for %s\n", fileOutname);
    }
    if (fileDes < 0){
      fprintf(stderr, "Can't open %s\n",fileOutname);
    }
    src = mmap(0, chunkSize, PROT_READ, MAP_SHARED, fileDes, 0);;
    if (src == MAP_FAILED){
      fprintf(stderr,"mmapping for %s failed\n",fileOutname);
    }
    buffer = (char*) src;
    if (fwrite(buffer, sizeof(char), chunkSize, stdout)==-1)
      {
	fprintf(stderr,"write to stdout failed\n");
      }      
    if (munmap(src, chunkSize) == -1) 
      {
	fprintf(stderr,"munmap for %s failed\n",fileOutname);
      }
    close(fileDes);
    i+=chunkSize;
    ++k;
  }
  return;
}

