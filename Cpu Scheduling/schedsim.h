#ifndef SCHEDSIM_H
#define SCHEDSIM_H

#include "list.h"
#include <sys/types.h>


struct task_struct {
  volatile long state; /* -1 unrunnable, 0 runnable, >0 stopped */
  unsigned int flags; /* per process flags, defined below */
  int on_rq;
  int tmp_src;
  int prio, static_prio, normal_prio;
  const struct sched_class *sched_class;
  struct list_head tasks;
  pid_t pid;
  int arr;
  /* simplify accounting */
  int ticks;
  int start_tick;
  int end_tick;
  int burst;
};

/*
 *  * This is the main, per-CPU runqueue data structure.
 */
struct rq {
  struct task_struct *curr, *idle, *stop;
  struct list_head task_root;
};


#endif /* SCHEDSIM_H */
