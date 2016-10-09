#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{
  if(argc != 1){
    printf(2, "Usage: check\n");
    exit();
  }
  struct pstat processState;
  getpinfo(&processState);
  printf (1, "%d\n", processState.inuse[0]);
  exit();
}
