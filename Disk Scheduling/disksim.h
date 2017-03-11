#ifndef DISKSIM_H
#define DISKSIM_H

#include "list.h"
#include <sys/types.h>


struct task_struct {
  int on_rq;
  int tmp_src;
  int cylinder, dist, up;
  struct list_head tasks;
};

/*
 *  * This is the main, per-CPU runqueue data structure.
 */
struct rq {
  struct task_struct *curr, *idle, *stop;
  struct list_head task_root;
};


#endif /* DISKSIM_H */
