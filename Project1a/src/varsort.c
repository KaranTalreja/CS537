#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"

int decompileArray (struct __rec_dataptr_t* inArray, unsigned int size);

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
  if (rc != sizeof(recordsLeft))
  {
    perror("read");
    exit(1);
  }
  printf("Number of records: %d\n", recordsLeft);
  rec_nodata_t r;
  unsigned int count = 0;
  struct __rec_dataptr_t* unsortedArray = (struct __rec_dataptr_t*)malloc(recordsLeft * sizeof(struct __rec_dataptr_t));
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

    unsortedArray[count].key = r.key;
    unsortedArray[count].data_ints = r.data_ints;
    unsortedArray[count].data_ptr = (unsigned int*)malloc(r.data_ints*sizeof(unsigned int));

    // Read the variable portion of the record
    rc = read(fd, unsortedArray[count].data_ptr, r.data_ints * sizeof(unsigned int));
    if (rc !=  r.data_ints * sizeof(unsigned int))
    {
      perror("read");
      exit(1);
    }

    recordsLeft--;
    count++;
  }

//  decompileArray(unsortedArray, count);

  (void)close(fd);
  (void)close(fdOut);
  return 0;
}

int decompileArray (struct __rec_dataptr_t* inArray, unsigned int size)
{
	if (NULL == inArray) return 1;
	unsigned int i = 0;
	while (i < size)
	{
	    printf("key %d: %u data_ints: %u rec: ", i, inArray[i].key, inArray[i].data_ints);
	    int j;
	    for (j = 0; j < inArray[i].data_ints; j++)
	      printf("%u ", *(inArray[i].data_ptr + j));
	    printf("\n");
	    i++;
	}
	return 0;
}
