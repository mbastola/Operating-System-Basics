#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "schedsim.h"
#include <sched.h>

/* forward declarations */
void fcfs_tick(struct rq *rq, struct task_struct *p, int queued);
void sjf_tick(struct rq *rq, struct task_struct *p, int queued);
void setup_scheduler(char *sched_name);
void setup_options(char *options);
void read_config(const char *config_file);
void run_simulator();

/* globals */
int tick=0;
int processes=0;
int span = 0; /*total burst time*/
void (*task_tick) (struct rq *rq, struct task_struct *p, int queued);
struct rq srcrq;
int* output;
struct rq runqueue;
#define MAX_BUFFER 500

// sample tick method
void fcfs_tick(struct rq *rq, struct task_struct *p, int queued)
{
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *entry;
  int i;
  int processAdded = 0;
  for (i = 0; i < span; i++)
    {
      if (processAdded == processes)
	return;
      list_for_each(ptr, &(srcrq.task_root)) {
	entry = list_entry(ptr, struct task_struct, tasks);
	if (entry->arr == i){
	  ts = (struct task_struct *)malloc(sizeof(struct task_struct));
	  ts->pid = entry->pid;
	  ts->arr = entry->arr;
	  ts->burst = entry->burst;
	  ts->prio = entry->prio;
	  list_add_tail(&(ts->tasks), &(rq->task_root));
	  processAdded++;
	}
      }
    }
  return;
}

void sjf_tick(struct rq *rq, struct task_struct *p, int queued)
{
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *entry;
  int i;
  int k = 0; //counts the total span of job in the run queue.
  int processAdded = 0;
  for ( i = 0; i < span ; ++i)
    {
      if (processAdded == processes)
	return;
      int j = 0; //counts of the span of jobs on the source queue 
      list_for_each(ptr, &(srcrq.task_root)) {
	entry = list_entry(ptr, struct task_struct, tasks);
	if (!entry->on_rq ){
	  j+=(entry->burst);
	  /*if burst is smaller than burst span counter i and it arrives after all the previous jobs are completed
	  or a job arrives when no other jobs are contenders*/ 
	  if ((entry->burst < i && entry->arr <= k ) || (i == j && entry->arr < i )){
	    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
	    ts->pid = entry->pid;
	    ts->arr = entry->arr;
	    ts->burst = entry->burst;
	    ts->prio = entry->prio;
	    entry->on_rq = 1;
	    list_add_tail(&(ts->tasks), &(rq->task_root));
	    k+=(entry->burst);
	    processAdded++;
	  }
	}
      }
    }
}

void stcf_tick(struct rq *rq, struct task_struct *p, int queued)
{
  struct task_struct *ts;
  struct list_head *ptr;
  struct task_struct *ts2;
  struct list_head *ptr2;
  struct task_struct *entry;
  struct task_struct *entry2;
  int i;
  int k = 0;  //counts the total span of job in the run queue.
  int processAdded = 0;
  int flag =0;
  int m = 0;  //counts the burst of broken down processes
  for (i = 0; i < span; ++i)
    {
      int j= 0; //counts of the span of jobs on the source queue 
      list_for_each(ptr, &(srcrq.task_root)) {
	entry = list_entry(ptr, struct task_struct, tasks);
	if (!entry->on_rq)
	  {
	    j+=entry->burst;
	     /*if burst is smaller than burst span counter i and it arrives after all the previous jobs are completed
	       or a job arrives when no other jobs are contenders*/
	    if ((entry->burst < i && entry->arr <= k) || (i == j && entry->arr < i)){
	      list_for_each(ptr2, &(rq->task_root)) 
		{ 
		  entry2 = list_entry(ptr2, struct task_struct, tasks);
		  if (entry->burst < entry2->burst && !(entry->tmp_src))
		    {
		      flag = 1;
		      break;
		    }
		  m+=entry2->burst;
	      }
	      if (m<entry->arr)
		m = (entry->arr);
	      ts = (struct task_struct *)malloc(sizeof(struct task_struct));
	      ts->pid = entry->pid;
	      ts->arr = entry->arr;
	      ts->burst = entry->burst;
	      ts->prio = entry->prio;
	      if (flag){
		//printf("Adding to head pid %d burst%d \n", ts->pid,ts->burst); 
		list_add(&(ts->tasks), ptr2);
	      }
	      else{
		//printf("Adding to tail pid %d burst%d \n", ts->pid,ts->burst); 
		list_add_tail(&(ts->tasks), &(rq->task_root));
	      }
	      entry->on_rq = 1;
	      k+=entry->burst;
	      processAdded++;
	      if (processAdded == processes)
		return;
	      if (flag){
		ts2 = (struct task_struct *)malloc(sizeof(struct task_struct));  
		ts2->pid = entry2->pid;
		ts2->arr = entry2->arr;
		ts2->burst = entry2->burst - m;
		//Breaking pid 
		//printf("breaking pid %d burst%d \n", ts2->pid,entry2->burst);
		ts2->prio = entry2->prio;
		ts2->on_rq = 0;
		ts2->tmp_src = 1;
		entry2->burst= entry->arr+1;
		//Rearranging the broken pid
		//printf("Adding to SRC pid %d burst%d \n", ts2->pid,ts2->burst);
		list_add_tail(&(ts2->tasks), ptr);
		flag = 0;
		processes++;
		m = 0;}
	    }
	  }
      }
    }
  return;
}

void run_simulator()
{
  // call the task_tick function for the scheduler that's loaded
  struct list_head *ptr;
  struct task_struct *entry;
  int j= 1;
  int k = 0;
  // printf("numProcesses %d \n", processes);
  if (task_tick != 0){
    task_tick(&runqueue,runqueue.curr,0);
    list_for_each(ptr, &(runqueue.task_root)) {
      entry = list_entry(ptr, struct task_struct, tasks);
      int i = entry->burst;
      while (i>0)
	{
	printf(",%d ",entry->pid);
	i--;
	j++;
	}
      output[k] = entry->pid;
      output[k+1] = j - entry->arr - entry->burst ;
      output[k+2] = j - entry->arr ;
      k+=3;
    }
  }
  printf("\n");
  int i = 0;
  for (i = 0; i < 3*processes; i+=3)
    printf("%d, %d, %d \n", output[i], output[i+1], output[i+2]);
}


void setup_scheduler(char *sched_name)
{
  /* Configure your schedulers here */
  if (strncmp(sched_name,"FCFS",4) == 0)
    {
      //printf("running sample First-Come-First-Serve\n");
      task_tick = &fcfs_tick;
    }
  else if (strncmp(sched_name,"SJF",3) == 0)
    {
      //printf("running sample First-Come-First-Serve\n");
      task_tick = &sjf_tick;
    }
  else if (strncmp(sched_name,"STCF",4) == 0)
    {
      //printf("running sample First-Come-First-Serve\n");
      task_tick = &stcf_tick;
    }
  /* initialize the run queue */
  INIT_LIST_HEAD(&(srcrq.task_root));
  INIT_LIST_HEAD(&(runqueue.task_root));
  //printf("inited\n");
}

void setup_options(char *options)
{
  //if (strcmp(options,""
}

char *read_line(char *buffer, FILE *fp)
{
  char *result = fgets(&buffer[0],MAX_BUFFER,fp);
  return result;
}


void print_rq(struct rq *rq)
{
  struct list_head *ptr;
  struct task_struct *entry;
  list_for_each(ptr, &(rq->task_root)) {
      entry = list_entry(ptr, struct task_struct, tasks);
      printf("pid %d, burst %d, prio %d arr %d\n",entry->pid, entry->burst, entry->prio, entry->arr);
  }
}


void read_config(const char *config_file)
{
  char buffer[MAX_BUFFER];
  char *tid, *arr, *burst, *prio;
  FILE *fp = fopen(config_file, "r");
  struct task_struct *ts;
  while(read_line(&buffer[0], fp) != NULL) {
    if (strnlen(&buffer[0],MAX_BUFFER)<=1) {
      printf("skipping empty line\n");
      continue;
    } 
    tid = strtok(buffer,",");
    arr = strtok(NULL,",");
    burst = strtok(NULL,",");
    prio = strtok(NULL,",");
    ts = (struct task_struct *)malloc(sizeof(struct task_struct));
    ts->pid = atoi(tid);
    ts->arr = atoi(arr);
    ts->burst = atoi(burst);
    ts->prio = atoi(prio);
    span += ts->burst;
    processes++;
    list_add_tail(&(ts->tasks), &(srcrq.task_root));
  }
  output = malloc(3*processes);
  fclose(fp);
  //printf("finished reading config file\n");
}

int main(int argc, char **argv)
{
  int opt;
  char schedulerName[256];
  char filename[256];

  if (argc < 3) {
    printf("usage:\nschedsim -s <scheduler> -i <input file>\n\n");
    return 0;
  }

  while ((opt = getopt(argc, argv, "s:i:")) != -1) {
    switch (opt) {
      case 's':
        strncpy(&schedulerName[0], optarg, 256);
        break;
      case 'i':
        strncpy(&filename[0], optarg, 256);
        break;
      default:                /* '?' */
        fprintf(stderr, "Usage: -s scheduler -i input_file\n");
        exit(EXIT_FAILURE);
    }
  }
  setup_scheduler(&schedulerName[0]);
  //printf("reading config_file %s\n",&filename[0]);
  read_config(&filename[0]);
  // print out the loaded queue
  // print_rq(&srcrq);
  printf(&schedulerName[0], " ");
  // run the simulator
  run_simulator();
  //print_rq(&runqueue);
  return 0;
}

