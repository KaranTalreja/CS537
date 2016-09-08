#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"

struct listNode
{
	struct listNode* next;
	struct __rec_dataptr_t* info;
};
struct list
{
	struct listNode* head;
	unsigned int length;
};

void usage(char *prog)
{
  fprintf(stderr, "usage: %s <-i file>\n", prog);
  exit(1);
}

int main(int argc, char *argv[])
{
  // arguments
  char *inFile = "/no/such/file";
  char *outFile = "/no/such/file";
  int c;

  opterr = 0;
  while (-1 != (c = getopt(argc, argv, "i:o:")))
  {
    switch (c) {
    case 'i':
      inFile = strdup(optarg);
      break;
    case 'o':
      outFile = strdup(optarg);
      break;
    default:
      usage(argv[0]);
    }
  }

  // open input file
  int fd = open(inFile, O_RDONLY);
  if (0 > fd)
  {
    perror("open");
    exit(1);
  }

  // open output file
  int fdOut = open(outFile, O_RDONLY);
  if (0 > fdOut)
  {
    perror("open");
    exit(1);
  }

  // output the number of keys as a header for this file
  int recordsLeft;
  int rc;

  rc = read(fd, &recordsLeft, sizeof(recordsLeft));
  if (rc != sizeof(recordsLeft)) {
    perror("read");
    exit(1);
  }
  int count = 0;
  printf("Number of records: %d\n", recordsLeft);
  rec_nodata_t r;
  unsigned int data[MAX_DATA_INTS];

  while (recordsLeft)
  {
    // Read the fixed-sized portion of record: key and size of data
    rc = read(fd, &r, sizeof(rec_nodata_t));
    if (rc != sizeof(rec_nodata_t))
    {
      perror("read");
      exit(1);
    }
    assert(r.data_ints <= MAX_DATA_INTS);

    // Read the variable portion of the record
    rc = read(fd, &data, r.data_ints * sizeof(unsigned int));
    if (rc !=  r.data_ints * sizeof(unsigned int))
    {
      perror("read");
      exit(1);
    }

//    printf("key %d: %u data_ints: %u rec: ", count, r.key, r.data_ints);
//    int j;
//    for (j = 0; j < r.data_ints; j++)
//      printf("%u ", data[j]);
//    printf("\n");

    recordsLeft--;
    count++;
  }

  (void)close(fd);
  (void)close(fdOut);
  return 0;
}