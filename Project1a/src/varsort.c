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
	struct listNode* first;
	struct listNode* current;
	unsigned int length;
};

int listAddNode (struct list* inList, struct listNode* appendNode);
int decompileList (struct list* inList);

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
//  unsigned int data[MAX_DATA_INTS];
  struct list* unsortedList = (struct list*)malloc(sizeof(struct list));
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

    struct listNode* currRecord = (struct listNode*)malloc(sizeof(struct listNode));
    currRecord->next = NULL;
    currRecord->info = (struct __rec_dataptr_t*)malloc(sizeof(struct __rec_dataptr_t));
    currRecord->info->key = r.key;
    currRecord->info->data_ints = r.data_ints;
    currRecord->info->data_ptr = (unsigned int*)malloc(r.data_ints*sizeof(unsigned int));

    // Read the variable portion of the record
    rc = read(fd, currRecord->info->data_ptr, r.data_ints * sizeof(unsigned int));
    if (rc !=  r.data_ints * sizeof(unsigned int))
    {
      perror("read");
      exit(1);
    }

    rc = listAddNode(unsortedList, currRecord);
    if (rc != 0)
    {
      perror("List Addition");
      exit(1);
    }

    recordsLeft--;
  }

  //decompileList(unsortedList);

  (void)close(fd);
  (void)close(fdOut);
  return 0;
}

int listAddNode (struct list* inList, struct listNode* appendNode)
{
	if ((NULL == inList) || (NULL == appendNode)) return 1;
	if (NULL == inList->first)
	{
		inList->first = appendNode;
		inList->current = appendNode;
		inList->length = 1;
	}
	else
	{
		inList->current->next = appendNode;
		inList->current = appendNode;
		inList->length++;
	}
	return 0;
}

int decompileList (struct list* inList)
{
	if ((NULL == inList) || (NULL == inList->first)) return 1;
	inList->current = inList->first;
	unsigned int count = 0;
	while (NULL != inList->current)
	{
		struct listNode* currRecord = inList->current;
	    printf("key %d: %u data_ints: %u rec: ", count++, currRecord->info->key, currRecord->info->data_ints);
	    int j;
	    for (j = 0; j < currRecord->info->data_ints; j++)
	      printf("%u ", *(currRecord->info->data_ptr + j));
	    printf("\n");
		inList->current = inList->current->next;
	}
	return 0;
}
