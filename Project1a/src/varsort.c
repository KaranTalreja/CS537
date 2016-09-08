#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"

int decompileArray (struct __rec_dataptr_t** inArray, unsigned int size);
int compareRecDataPtr (__const void* first, __const void* second);

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
  int fdOut = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
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
  struct __rec_dataptr_t** unsortedArray = (struct __rec_dataptr_t**)malloc(recordsLeft * sizeof(struct __rec_dataptr_t*));
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

    unsortedArray[count] = (struct __rec_dataptr_t*)malloc(sizeof(struct __rec_dataptr_t));
    unsortedArray[count]->key = r.key;
    unsortedArray[count]->data_ints = r.data_ints;
    unsortedArray[count]->data_ptr = (unsigned int*)malloc(r.data_ints*sizeof(unsigned int));

    // Read the variable portion of the record
    rc = read(fd, unsortedArray[count]->data_ptr, r.data_ints * sizeof(unsigned int));
    if (rc !=  r.data_ints * sizeof(unsigned int))
    {
      perror("read");
      exit(1);
    }

    recordsLeft--;
    count++;
  }

//  decompileArray(unsortedArray, count);

  qsort(&unsortedArray[0], count, sizeof(struct __rec_dataptr_t*), compareRecDataPtr);

//  decompileArray(unsortedArray, count);
  // output the number of keys as a header for this file
  rc = write(fdOut, &count, sizeof(count));
  if (rc != sizeof(count))
  {
    perror("write");
    exit(1);
  }
  unsigned int i = 0;
  struct __rec_data_t dumpRec;
  while (i < count)
  {
	  dumpRec.key = unsortedArray[i]->key;
	  dumpRec.data_ints = unsortedArray[i]->data_ints;
	  unsigned int j;
	  for (j = 0; j < unsortedArray[i]->data_ints; j++)
		  dumpRec.data[j] = *(unsortedArray[i]->data_ptr + j);
	  i++;
	  int data_size = 2*sizeof(unsigned int) + dumpRec.data_ints*sizeof(unsigned int);

	  rc = write(fdOut, &dumpRec, data_size);

	  if (rc != data_size )
	  {
	    perror("write");
	    exit(1);
	  }
  }


  (void)close(fd);
  (void)close(fdOut);
  return 0;
}

int decompileArray (struct __rec_dataptr_t** inArray, unsigned int size)
{
	if (NULL == inArray) return 1;
	unsigned int i = 0;
	while (i < size)
	{
	    printf("key %d: %u data_ints: %u rec: ", i, inArray[i]->key, inArray[i]->data_ints);
	    int j;
	    for (j = 0; j < inArray[i]->data_ints; j++)
	      printf("%u ", *(inArray[i]->data_ptr + j));
	    printf("\n");
	    i++;
	}
	return 0;
}

int compareRecDataPtr (__const void* first, __const void* seccond)
{
	struct __rec_dataptr_t* firstRecord = *(struct __rec_dataptr_t**)first;
	struct __rec_dataptr_t* secondRecord = *(struct __rec_dataptr_t**)seccond;
	if (firstRecord->key < secondRecord->key) return -1;
	else if (firstRecord->key > secondRecord->key) return 1;
	return 0;
}
