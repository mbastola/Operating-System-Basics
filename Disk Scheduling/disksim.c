#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "disksim.h"
#include <sched.h>

/* forward declarations */
void read_config(FILE* fp);
void print_rq(FILE* fp,struct rq *rq);
int Up_or_Down();
char *read_line(char *buffer, FILE *fp);
int sstf_compare(struct list_head *a, struct list_head *b);
int scan_compare(struct list_head *a, struct list_head *b);
int cscan_compare(struct list_head *a, struct list_head *b);
int look_compare(struct list_head *a, struct list_head *b);
int clook_compare(struct list_head *a, struct list_head *b);
void fcfs_tick(struct rq *rq, FILE* fp);
void sstf_tick(struct rq *rq, FILE* fp);
void scan_tick(struct rq *rq, FILE* fp);
void cscan_tick(struct rq *rq, FILE* fp);
void look_tick(struct rq *rq, FILE* fp);
void clook_tick(struct rq *rq, FILE* fp);

/* globals */
int start;
int processes;
#define LOWEST_LEVEL 0
#define HIGHEST_LEVEL 200
#define MAX_BUFFER 500
int elevatorUp;
void (*task_tick) (struct rq *rq, struct task_struct *p, int queued);
struct rq srcrq;
int* output;
struct rq runqueue;



void fcfs_tick(struct rq *rq, FILE* fp)
{
  INIT_LIST_HEAD(&(rq->task_root));
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *entry;
  int totalDist = 0;
  int prevDist = start;
  list_for_each(ptr, &(srcrq.task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
    ts->cylinder = entry->cylinder;
    totalDist+=abs(prevDist-ts->cylinder);
    prevDist = ts->cylinder;
    list_add_tail(&(ts->tasks), &(rq->task_root));
  }
  fprintf(fp,"FCFS,%d,",totalDist);
  print_rq(fp,rq);
}
  
void sstf_tick(struct rq *rq, FILE* fp)
{
  INIT_LIST_HEAD(&(rq->task_root));
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *entry;
  int totalDist = 0;
  int prevDist = start;
  list_for_each(ptr, &(srcrq.task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
    ts->cylinder = entry->cylinder;
    ts->dist = entry->dist;
    list_add_tail(&(ts->tasks), &(rq->task_root));
  }
  list_sort(&(rq->task_root),*sstf_compare);
  list_for_each(ptr, &(rq->task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    totalDist+=abs(prevDist-entry->cylinder);
    prevDist = entry->cylinder;
  }
  fprintf(fp,"SSTF,%d,",totalDist);
  print_rq(fp,rq);
}

void scan_tick(struct rq *rq, FILE* fp)
{
  INIT_LIST_HEAD(&(rq->task_root));
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *entry;
  int totalDist = 0;
  int prevDist = start;
  ts = (struct task_struct *)malloc(sizeof(struct task_struct));
  ts->cylinder = LOWEST_LEVEL;
  ts->dist = start;
  ts->up = 0;
  list_add_tail(&(ts->tasks), &(rq->task_root));
  ++processes;
  list_for_each(ptr, &(srcrq.task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
    ts->cylinder = entry->cylinder;
    ts->dist = entry->dist;
    ts->up = entry->up;
    list_add_tail(&(ts->tasks), &(rq->task_root));
  }
  ts = (struct task_struct *)malloc(sizeof(struct task_struct));
  ts->cylinder = HIGHEST_LEVEL;
  ts->dist = HIGHEST_LEVEL - start;
  ts->up = 1;
  list_add_tail(&(ts->tasks), &(rq->task_root));
  ++processes;
  elevatorUp = Up_or_Down();
  list_sort(&(rq->task_root),*scan_compare);
  list_for_each(ptr, &(rq->task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    //    if (entry->cylinder){
      totalDist+=abs(prevDist-entry->cylinder);
      prevDist = entry->cylinder;
      // }
  }
  fprintf(fp,"SCAN,%d,",totalDist);
  print_rq(fp,rq);
  processes-=2;
}

void cscan_tick(struct rq *rq, FILE* fp)
{
  INIT_LIST_HEAD(&(rq->task_root));
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *entry;
  int totalDist = 0;
  int prevDist = start;
  int directionChange = 0;
  ts = (struct task_struct *)malloc(sizeof(struct task_struct));
  ts->cylinder = LOWEST_LEVEL;
  ts->dist = start;
  ts->up = 0;
  list_add_tail(&(ts->tasks), &(rq->task_root));
  ++processes;
  list_for_each(ptr, &(srcrq.task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
    ts->cylinder = entry->cylinder;
    ts->dist = entry->dist;
    ts->up = entry->up;
    list_add_tail(&(ts->tasks), &(rq->task_root));
  }
  ts = (struct task_struct *)malloc(sizeof(struct task_struct));
  ts->cylinder = HIGHEST_LEVEL;
  ts->dist = HIGHEST_LEVEL - start;
  ts->up = 1;
  list_add_tail(&(ts->tasks), &(rq->task_root));
  ++processes;
  elevatorUp = Up_or_Down();
  list_sort(&(rq->task_root),*cscan_compare);
  list_for_each(ptr, &(rq->task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    if (entry->cylinder && !directionChange){
      totalDist+=abs(prevDist-entry->cylinder);
      prevDist = entry->cylinder;
    }
    if (entry->cylinder == LOWEST_LEVEL || entry->cylinder == HIGHEST_LEVEL )
      {
	directionChange=1;
      }
    if (directionChange){
      totalDist+=abs(prevDist+entry->cylinder);
      prevDist = entry->cylinder;
      directionChange = 0;
    }
  }
  fprintf(fp,"CSCAN,%d,",totalDist);
  print_rq(fp,rq);
  processes-=2;
}

void look_tick(struct rq *rq, FILE* fp)
{
  INIT_LIST_HEAD(&(rq->task_root));
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *entry;
  int totalDist = 0;
  int prevDist = start;
  list_for_each(ptr, &(srcrq.task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
    ts->cylinder = entry->cylinder;
    ts->dist = entry->dist;
    ts->up = entry->up;
    list_add_tail(&(ts->tasks), &(rq->task_root));
  }
  elevatorUp = Up_or_Down();
  list_sort(&(rq->task_root),*scan_compare);
  list_for_each(ptr, &(rq->task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    totalDist+=abs(prevDist-entry->cylinder);
    prevDist = entry->cylinder;
  }
  fprintf(fp,"LOOK,%d,",totalDist);
  print_rq(fp,rq);
}

void clook_tick(struct rq *rq, FILE* fp)
{
  INIT_LIST_HEAD(&(rq->task_root));
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *entry;
  int totalDist = 0;
  int prevDist = start;
  int directionChange = 0;
  list_for_each(ptr, &(srcrq.task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
    ts->cylinder = entry->cylinder;
    ts->dist = entry->dist;
    ts->up = entry->up;
    list_add_tail(&(ts->tasks), &(rq->task_root));
  }
  elevatorUp = Up_or_Down();
  list_sort(&(rq->task_root),*cscan_compare);
  list_for_each(ptr, &(rq->task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    if (entry->cylinder && !directionChange){
      totalDist+=abs(prevDist-entry->cylinder);
      prevDist = entry->cylinder;
    }
    if (!entry->cylinder)
      {
	directionChange=1;
      }
    if (directionChange){
      totalDist+=abs(prevDist+entry->cylinder);
      prevDist = entry->cylinder;
      directionChange = 0;
    }
  }
  fprintf(fp,"CLOOK,%d,",totalDist);
  print_rq(fp,rq);
}


int sstf_compare(struct list_head *a, struct list_head *b)
{
  struct task_struct *entry1;
  struct task_struct *entry2;
  entry1 = list_entry(a, struct task_struct, tasks);
  entry2 = list_entry(b, struct task_struct, tasks);
  return (entry1->dist>entry2->dist);
}

int scan_compare(struct list_head *a, struct list_head *b)
{
  struct task_struct *entry1;
  struct task_struct *entry2;
  entry1 = list_entry(a, struct task_struct, tasks);
  entry2 = list_entry(b, struct task_struct, tasks);
  if (entry1->up && entry2->up)
    return (entry1->dist>entry2->dist);
  else if (!entry1->up && !entry2->up)
    return (entry1->dist>entry2->dist);
  else if (!entry1->up && entry2->up)
    return elevatorUp;
  else
    return !elevatorUp;
}

int cscan_compare(struct list_head *a, struct list_head *b)
{
  struct task_struct *entry1;
  struct task_struct *entry2;
  entry1 = list_entry(a, struct task_struct, tasks);
  entry2 = list_entry(b, struct task_struct, tasks);
  if (!elevatorUp){
    if (entry1->up && entry2->up)
      return (entry1->dist<entry2->dist);
    else if (!entry1->up && !entry2->up)
      return (entry1->dist>entry2->dist);  /*Circular for elevator going up*/
    else if (!entry1->up && entry2->up)
      return elevatorUp;
    else
      return !elevatorUp;
  }
  else{
    if (entry1->up && entry2->up)
      return (entry1->dist>entry2->dist);         /*Circular for elevator going down*/
    else if (!entry1->up && !entry2->up)
      return (entry1->dist<entry2->dist);
    else if (!entry1->up && entry2->up)
      return 1;
    else
      return 0;
  }
}

int Up_or_Down()
{
  srand(time(NULL));
  int randomBit = rand() & 1;
  return randomBit;
}

char *read_line(char *buffer, FILE *fp)
{
  char *result = fgets(&buffer[0],MAX_BUFFER,fp);
  return result;
}


void print_rq(FILE* fp,struct rq *rq)
{
  struct list_head *ptr;
  struct task_struct *entry;
  int i=0;
  list_for_each(ptr, &(rq->task_root)) {
    entry = list_entry(ptr, struct task_struct, tasks);
    ++i;
    if(i == processes)
      fprintf(fp,"%d\n",entry->cylinder);
    else
      fprintf(fp,"%d,",entry->cylinder);
  }
}

void read_config(FILE *fp)
{
  char buffer[MAX_BUFFER];
  char *val;
  struct task_struct *ts;
  read_line(&buffer[0], fp);
  if (strnlen(&buffer[0],MAX_BUFFER)<=1) {
    printf("Empty file\n");
    return;
  }
  val = strtok(buffer,":");
  val = strtok(NULL,":");
  start = atoi(val);
  processes = 0;
  while(read_line(&buffer[0], fp) != NULL) {
    if (strnlen(&buffer[0],MAX_BUFFER)<=1) {
      printf("skipping empty line\n");
      continue;
    }
    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
    ts->cylinder = atoi(buffer);
    int dist = ts->cylinder-start;
    if (dist>0)
      ts->up = 1;
    else
      ts->up = 0;
    ts->dist = abs(dist); 
    list_add_tail(&(ts->tasks), &(srcrq.task_root));
    ++processes;
  }
}

int main(int argc, char **argv)
{
  int opt;
  char infileName[256];
  char outfileName[256];
  int outFlag = 0;
  if (argc < 3) {
    printf("usage:./disksim -i <input file> -o <output file>\n\n");
    return 0;
  }

  while ((opt = getopt(argc, argv, "o:i:")) != -1) {
    switch (opt) {
    case 'o':
      strncpy(&outfileName[0], optarg, 256);
      outFlag = 1;
      break;
    case 'i':
      strncpy(&infileName[0], optarg, 256);
      break;
    default:                /* '?' */
      fprintf(stderr, "Usage:./disksim -i <input file> -o <output file>\n");
      exit(EXIT_FAILURE);
    }
  }
  INIT_LIST_HEAD(&(srcrq.task_root));
  FILE *infp = fopen(infileName, "r");
  if (infp == NULL){
    fprintf(stderr,"File couldn't be read\n");
    exit(EXIT_FAILURE);
  }
  read_config(infp);
  fclose(infp);
  FILE *outfp = stdout;
  if (outFlag)
    outfp = fopen(outfileName, "w");
  fcfs_tick(&runqueue,outfp);
  sstf_tick(&runqueue,outfp);
  scan_tick(&runqueue,outfp);
  cscan_tick(&runqueue,outfp);
  look_tick(&runqueue,outfp);
  clook_tick(&runqueue,outfp);
  fclose(outfp);
  return 0;
}

