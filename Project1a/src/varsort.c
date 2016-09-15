#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "sort.h"

#define BASE 256

typedef struct __element {
  rec_nodata_t *rec_nodata;
  unsigned int *data_ptr;
} element;

int decompileArray(element** inArray, unsigned int size);
int compareRecDataPtr(const void* first, const void* second);
int radixSort(element** array, unsigned int size);

void usage() {
  fprintf(stderr, "Usage: varsort -i inputfile -o outputfile\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  // arguments
  char *inFile = NULL;
  char *outFile = NULL;
  int c;
  if (argc < 5) usage();
  opterr = 0;
  while (-1 != (c = getopt(argc, argv, "i:o:"))) {
    switch (c) {
      case 'i':
        inFile = strdup(optarg);
        break;
      case 'o':
        outFile = strdup(optarg);
        break;
      default:
        usage();
    }
  }
  if ((NULL == inFile) || (NULL == outFile)) usage();
  // open input file
  int fd = open(inFile, O_RDONLY);
  if (0 > fd) {
    fprintf(stderr, "Error: Cannot open file %s\n", inFile);
    exit(1);
  }

  // open output file
  int fdOut = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
  if (0 > fdOut) {
    fprintf(stderr, "Error: Cannot open file %s\n", outFile);
    exit(1);
  }

  struct stat inFileStat;
  fstat(fd, &inFileStat);
  size_t sizeOfFile = inFileStat.st_size;

  // output the number of keys as a header for this file
  int recordsLeft;
  int rc;

  rc = read(fd, &recordsLeft, sizeof(recordsLeft));
  if (rc != sizeof(recordsLeft)) {
    fprintf(stderr, "read error\n");
    exit(1);
  }

  sizeOfFile = sizeOfFile - sizeof(unsigned int);
  unsigned int count = 0;
  size_t sizeOfOMContainer = recordsLeft*sizeof(element*);
  size_t sizeOfOMElements = recordsLeft*sizeof(element);
  void *memoryPool = malloc(sizeOfFile+sizeOfOMContainer+sizeOfOMElements);
  void *memoryPoolToFree = memoryPool;
  element** array = (element**)(memoryPool+sizeOfFile);
  void *memoryPoolOfOMElements = memoryPool + sizeOfFile + sizeOfOMContainer;
  rc = read(fd, memoryPool, sizeOfFile);
  if (rc != sizeOfFile) {
    fprintf(stderr, "read error\n");
    exit(1);
  }

  while (recordsLeft) {
    array[count] = (element*)memoryPoolOfOMElements;
    memoryPoolOfOMElements += sizeof(element);
    array[count]->rec_nodata = (rec_nodata_t*)memoryPool;
    memoryPool += sizeof(rec_nodata_t);
    array[count]->data_ptr = (unsigned int*)memoryPool;
    memoryPool += array[count]->rec_nodata->data_ints*sizeof(unsigned int);
    recordsLeft--;
    count++;
  }

  // qsort(array, count, sizeof(struct __element*), compareRecDataPtr);

  radixSort(array, count);

  // output the number of keys as a header for this file
  rc = write(fdOut, &count, sizeof(count));
  if (rc != sizeof(count)) {
    fprintf(stderr, "write error\n");
    exit(1);
  }

  unsigned int i = 0;
  while (i < count) {
    int data_size = (2+array[i]->rec_nodata->data_ints)*sizeof(unsigned int);
    rc = write(fdOut, array[i]->rec_nodata, data_size);
    i++;
    if (rc != data_size) {
      fprintf(stderr, "write error\n");
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

int compareRecDataPtr(const void* first, const void* seccond) {
  unsigned int firstRecordKey = (*(element**)first)->rec_nodata->key;
  unsigned int secondRecordKey = (*(element**)seccond)->rec_nodata->key;
  if (firstRecordKey < secondRecordKey) return -1;
  else if (firstRecordKey > secondRecordKey) return 1;
  return 0;
}

int radixSort(element** array, unsigned int size) {
  if ((NULL == array) || (0 == size)) return 1;
  int maxDigits = 0;
  unsigned int exp = 1;
  unsigned int maxKey = array[0]->rec_nodata->key;
  unsigned int i = 0;
  for (i = 1; i < size ; i++) {
    if (array[i]->rec_nodata->key > maxKey) maxKey = array[i]->rec_nodata->key;
  }
  while (0 != maxKey) {
    maxDigits++;
    maxKey /= BASE;
  }
  int j = 0;
  element** arrayTmp = (element**) malloc(size*sizeof(element*));
  for (j = 0; j < maxDigits; j++) {
    int counters[BASE] = {0};
    for (i = 0 ; i < size; i++) {
      counters[(array[i]->rec_nodata->key / exp) % BASE]++;
    }
    for (i = 1 ; i < BASE; i++) {
      counters[i] = counters[i] + counters[i-1];
    }
    for (i = size ; i > 0; i--) {
      arrayTmp[--counters[(array[i-1]->rec_nodata->key/exp)%BASE]] = array[i-1];
    }
    for (i = 0 ; i < size; i++) {
      array[i] = arrayTmp[i];
    }
    exp = exp * BASE;
  }
  free(arrayTmp);
  return 0;
}
