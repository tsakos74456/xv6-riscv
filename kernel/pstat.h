#ifndef XV6_PSTAT_H
#define XV6_PSTAT_H

#include "param.h"

struct pstat {
  int num_processes;
  int pid[NPROC];
  int ppid[NPROC];
  char name[NPROC][16];
  int state[NPROC];
  int size[NPROC];
  int priority[NPROC];
  int in_use[NPROC];
};

#endif
