#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

int
main(int argc, char *argv[])
{
  void* mem = shmgetat(3,3);
  int ref_count = shm_refcount(3);
  printf(1, "%p, %d\n", mem, ref_count);
  int pid = fork();
  if (0 == pid)
  {
    ref_count = shm_refcount(3);
    printf(1, "CHILD:KEY 3:%p, %d\n", mem, ref_count);
    void* mem = shmgetat(3,2);
    ref_count = shm_refcount(3);
    printf(1, "CHILD:KEY 3:%p, %d\n", mem, ref_count);
    mem = shmgetat(4,1);
    ref_count = shm_refcount(4);
    printf(1, "CHILD:KEY 4:%p, %d\n", mem, ref_count);
  }
  else
  {
    printf(1, "PARENT:BC:%p, %d\n", mem, ref_count);
    wait();
    printf(1, "PARENT:AD:%p, %d\n", mem, ref_count);
  }
  exit();
}
