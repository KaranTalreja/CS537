#ifndef __STATS_H__
#define __STATS_H__

#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
  // You may add any new fields that you believe are necessary
  int pid;        // Do not remove or change
  int counter;    // Do not remove or change
  int priority;   // Do not remove or change
  double cpu_secs; // Do not remove or change
  // You may add any new fields that you believe are necessary
} stats_t;

stats_t* stat_init(key_t key);

int stat_unlink(key_t key);

#endif
