#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include "sort.h"

#define BASE 2048

/**
 * @brief Data structure to hold the record data in unsorted array
 * @details Contains two elements,
 * 1. rec_nodata_t pointer to memory containing key and number of data ints 
 * 2. unsigned int pointer to memory containing array of ints part of record.
 */
typedef struct __element {
  rec_nodata_t *rec_nodata;
  unsigned int *data_ptr;
} element;


// @brief Comparator function for qsort.
int compareRecDataPtr(const void* first, const void* second);

// @brief Function to implement radix sort.
int radixSort(element** array, unsigned int size);

// @brief Function to print out usage of this program in case of invalid args
void usage() {
  fprintf(stderr, "Usage: varsort -i inputfile -o outputfile\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  /**
  * Input arguments are parsed to get information about input file 
  * and output file.
  * Error message is flagged to stderr in case of following scenarios
  * 1. Executable receives less than 4 arguments
  * 2. Option other than -i, -o are provided to the program.
  * 3. No options are provided to the program.
  */
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

  // check file size for subsequent malloc call.
  struct stat inFileStat;
  fstat(fd, &inFileStat);
  size_t sizeOfFile = inFileStat.st_size;

  int recordsLeft;
  int rc;

  // Read number of records in the input file
  rc = read(fd, &recordsLeft, sizeof(recordsLeft));
  if (rc != sizeof(recordsLeft)) {
    fprintf(stderr, "read error\n");
    exit(1);
  }

  // size of file to be read.
  sizeOfFile = sizeOfFile - sizeof(unsigned int);

  // size of OM container.
  size_t sizeOfOMContainer = recordsLeft*sizeof(element*);

  // size of elements contained in the OM container.
  size_t sizeOfOMElements = recordsLeft*sizeof(element);

  size_t totalSizeOfMemory = sizeOfFile+sizeOfOMContainer+sizeOfOMElements;
  // Variable to hold all the memory allocated for file read and data storage
  void *memoryPool = malloc(totalSizeOfMemory);
  if (totalSizeOfMemory != 0 && NULL == memoryPool) {
    fprintf(stderr, "memory allocation error.\n");
    exit(0);
  }

  // Backup pointer to free memory in one call to free()
  void *memoryPoolToFree = memoryPool;

  /** OM container which lies at an offset of sizeOfFile from the start
  of memoryPool */
  element** array = (element**)(memoryPool+sizeOfFile);

  /** OM elements which will be pointed to by pointers in the OM container.
  * This memory lies at an offset of sizeOfFile + sizeOfOMContainer from
  * the start of the memoryPool.
  */
  void *memoryPoolOfOMElements = memoryPool + sizeOfFile + sizeOfOMContainer;

  // Read the contents of file into memoryPool whose size is sizeOfFile
  rc = read(fd, memoryPool, sizeOfFile);
  if (rc != sizeOfFile) {
    fprintf(stderr, "read error\n");
    exit(1);
  }

  // Loop to align all pointers to correct memory addresses in the memoryPool.
  unsigned int count = 0;
  while (recordsLeft) {
    /** Each element in array is a pointer to memory contained in 
    memoryPoolOfOMElements. Then the memory location is incremented for 
    subsequent assignment to next element. */
    array[count] = (element*)memoryPoolOfOMElements;
    memoryPoolOfOMElements += sizeof(element);

    /** Each element of array contains two pointers. First member points to
    rec_nodata_t type memory in memoryPool, which is where all file contents
    were read to. Similarly second member points to array of integer records
    in memoryPool. After there assignments the pointer memoryPool is
    incremented accordingly for subsequent assignments.*/
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

  /** File write loop to write each record to new file. Here only one call is
  used to write the complete record because the complete record lies at a
  contigious location in memory. The OM was designed to be able to reuse the
  input file structure. Each write call writes the key, number of data ints
  and ints in record to the output file.*/
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

/** The function implements the radix sort routine to be used in the program.
The funcion first determines the maximum key to learn the number of digits
in it. This number determines the number of loops which will be required
for sorting the data.
Second, the function allocates some memory to act as buffer for memory swapping
Third, it loops to create the sorted array.*/
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
  if (size != 0 && NULL == arrayTmp) {
    fprintf(stderr, "memory allocation error.\n");
    exit(0);
  }
  element** bfrArr = arrayTmp;
  element** primArr = array;
  for (j = 0; j < maxDigits; j++) {
    // Variable to hold number of keys which contain the particular digit.
    int counters[BASE] = {0};

    // Loop to increment counters specific to digits
    for (i = 0 ; i < size; i++) {
      counters[(primArr[i]->rec_nodata->key / exp) % BASE]++;
    }

    /** Loop to modify counters variable in such a way that now each index
    contains number of keys which contain a digit lower than current position
    digit.*/
    for (i = 1 ; i < BASE; i++) {
      counters[i] = counters[i] + counters[i-1];
    }

    /** Loop to assign unsorted array elements to buffer memory. This is done
    according to the following criteria. It is placed in the buffer according
    to the sorting of current digit. Then the counter contents are decreased,
    so that the next key with same digit will land into memory which has
    address one less than previous same digit key.*/
    for (i = size ; i > 0; i--) {
      bfrArr[--counters[(primArr[i-1]->rec_nodata->key/exp)%BASE]] =
      primArr[i-1];
    }

    if (bfrArr == arrayTmp) {
      bfrArr = array;
      primArr = arrayTmp;
    } else if (bfrArr == array) {
      bfrArr = arrayTmp;
      primArr = array;
    }
    exp = exp * BASE;
  }
  if (primArr != array) {
    // Assign back the contents of buffer memory to original array.
    for (i = 0 ; i < size; i++) {
      array[i] = primArr[i];
    }
  }
  free(arrayTmp);
  return 0;
}
