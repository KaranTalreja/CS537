#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include "stats.h"

// @brief Function to print out usage of this program in case of invalid args
void usage() {
  char* usageStr = \
  "Usage: stats_client -k key -p priority -s sleeptime_ns -c cputime_ns\n";
  write(STDERR_FILENO, usageStr, strlen(usageStr));
  exit(1);
}

int
main(int argc, char* argv[]) {
  if (argc > 9) usage();
  size_t key, priority = 0, sleeptime, cputime;
  opterr = 0;
  char c;
  while (-1 != (c = getopt(argc, argv, "k:p:s:c:"))) {
    switch (c) {
      case 'k':
        key = atoi(optarg);
        break;
      case 'p':
        priority = atoi(optarg);
        break;
      case 's':
        sleeptime = atoi(optarg);
        break;
      case 'c':
        cputime = atoi(optarg);
        break;
      default:
        usage();
    }
  }
  return 0;
}
