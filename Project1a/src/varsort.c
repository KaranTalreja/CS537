#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "sort.h"


typedef struct __rec_nodata_wrapper_t {
  rec_nodata_t *rec_nodata;
  unsigned int *data_ptr;
} rec_nodata_wrapper_t;

int decompileArray (struct __rec_nodata_wrapper_t** inArray, unsigned int size);
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

  struct stat inFileStat;
  fstat(fd, &inFileStat);
  size_t sizeOfFile = inFileStat.st_size;

  // output the number of keys as a header for this file
  int recordsLeft;
  int rc;

  rc = read(fd, &recordsLeft, sizeof(recordsLeft));
  if (rc != sizeof(recordsLeft))
  {
    perror("read");
    exit(1);
  }

  sizeOfFile = sizeOfFile - sizeof(unsigned int);
  printf("Number of records: %d\n", recordsLeft);
  unsigned int count = 0;
  size_t sizeOfOMContainer = recordsLeft*sizeof(struct __rec_nodata_wrapper_t*);
  size_t sizeOfOMElements = recordsLeft*sizeof(struct __rec_nodata_wrapper_t);
  void *memoryPool = malloc(sizeOfFile + sizeOfOMContainer + sizeOfOMElements);
  void *memoryPoolToFree = memoryPool;
  struct __rec_nodata_wrapper_t** unsortedArray = (struct __rec_nodata_wrapper_t**)(memoryPool + sizeOfFile);
  void *memoryPoolOfOMElements = memoryPool + sizeOfFile + sizeOfOMContainer;
  rc = read(fd, memoryPool, sizeOfFile);
  if (rc != sizeOfFile)
  {
    perror("read");
    exit(1);
  }
  while (recordsLeft)
  {
    unsortedArray[count] = (struct __rec_nodata_wrapper_t*)memoryPoolOfOMElements;
    memoryPoolOfOMElements += sizeof(struct __rec_nodata_wrapper_t);
    unsortedArray[count]->rec_nodata = (rec_nodata_t*)memoryPool;
    memoryPool += sizeof(rec_nodata_t);
    unsortedArray[count]->data_ptr = (unsigned int*)memoryPool;
    memoryPool += (unsortedArray[count]->rec_nodata->data_ints*sizeof(unsigned int));
    recordsLeft--;
    count++;
  }

//  decompileArray(unsortedArray, count);

  qsort(&unsortedArray[0], count, sizeof(struct __rec_nodata_wrapper_t*), compareRecDataPtr);

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
	  dumpRec.key = unsortedArray[i]->rec_nodata->key;
	  dumpRec.data_ints = unsortedArray[i]->rec_nodata->data_ints;
	  unsigned int j;
	  for (j = 0; j < dumpRec.data_ints ; j++)
		  dumpRec.data[j] = *(unsortedArray[i]->data_ptr + j);
	  i++;
	  int data_size = (2 + dumpRec.data_ints)*sizeof(unsigned int);

	  rc = write(fdOut, &dumpRec, data_size);

	  if (rc != data_size )
	  {
	    perror("write");
	    exit(1);
	  }
  }

  free(inFile);
  free(outFile);
  free(memoryPoolToFree);
  (void)close(fd);
  (void)close(fdOut);
  return 0;
}

int decompileArray (struct __rec_nodata_wrapper_t** inArray, unsigned int size)
{
	if (NULL == inArray) return 1;
	unsigned int i = 0;
	while (i < size)
	{
	    printf("key %d: %u data_ints: %u rec: ", i, inArray[i]->rec_nodata->key, inArray[i]->rec_nodata->data_ints);
	    int j;
	    for (j = 0; j < inArray[i]->rec_nodata->data_ints; j++)
	      printf("%u ", *(inArray[i]->data_ptr + j));
	    printf("\n");
	    i++;
	}
	return 0;
}

int compareRecDataPtr (__const void* first, __const void* seccond)
{
	unsigned int firstRecordKey = (*(struct __rec_nodata_wrapper_t**)first)->rec_nodata->key;
	unsigned int secondRecordKey = (*(struct __rec_nodata_wrapper_t**)seccond)->rec_nodata->key;
	if (firstRecordKey < secondRecordKey) return -1;
	else if (firstRecordKey > secondRecordKey) return 1;
	return 0;
}
