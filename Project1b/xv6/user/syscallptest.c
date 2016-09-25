#include "types.h"
#include "user.h"

int main(int argc, char *argv[])
{
  int i,sysCallCount, sysCallCountAfter;
  int N;
  if(argc != 2)
  {
    printf(2, "Usage: syscallptest N\n");
    exit();
  }
  sysCallCount = getnumsyscallp();
  N = atoi(argv[1]);
  for(i = 0; i < N; i++)
  {
    (void)(getpid());
  }
  sysCallCountAfter = getnumsyscallp();
  printf(1,"%d\n%d\n",sysCallCount,sysCallCountAfter);
  exit();
}
