#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/types.h>

typedef enum __jobType {
  FOREGROUND_JOB,
  BACKGROUND_JOB,
  BUILTIN_JOB
} jobType_e;

typedef enum __mode {
  BATCH_MODE,
  INTERACTIVE_MODE
} mode_e;

typedef enum __builtin {
  EXIT_CMD,
  J_CMD,
  MYW_CMD
} builtin_e;

typedef enum __jobStatus {
  STATUS_RUNNING,
  STATUS_TERMINATED
} jobStatus_e;

typedef struct __process {
  unsigned int jid;
  unsigned int pid;
  jobStatus_e status;
  char* command;
} process_t;

typedef struct __listnode {
  process_t* info;
  struct __listnode* next;
} listnode_t;

typedef struct __list {
  listnode_t* first;
  listnode_t* current;
} list_t;

// @brief Function to print out usage of this program in case of invalid args
void usage() {
  char* usageStr = "Usage: mysh [batchFile]\n";
  write(STDERR_FILENO, usageStr, strlen(usageStr));
  exit(1);
}

void
listAppendNode(list_t* list, process_t* process) {
  if ((NULL == list) || (NULL == process)) return;
  listnode_t* currNode = (listnode_t*)malloc(sizeof(listnode_t));
  currNode->info = process;
  currNode->next = NULL;
  if (NULL == list->first) {
    list->first = currNode;
    list->current = currNode;
  } else {
    list->current->next = currNode;
    list->current = currNode;
  }
  return;
}

void
decompileList(list_t* inList) {
  if ((NULL == inList) || (NULL == inList->first)) return;
  listnode_t *current = inList->first;
  while (NULL != current) {
    listnode_t* currRecord = current;
    if (STATUS_RUNNING == currRecord->info->status) {
      int status = 0;
      if (0 == waitpid(currRecord->info->pid, &status, WNOHANG)) {
        printf("%d : %s\n", currRecord->info->jid, currRecord->info->command);
      } else {
        currRecord->info->status = STATUS_TERMINATED;
      }
    }
    current = current->next;
  }
  return;
}

int
main(int argc, char* argv[]) {
  mode_e shellMode = INTERACTIVE_MODE;
  jobType_e jobType;
  builtin_e builtinType;
  list_t* processList = (list_t*)malloc(sizeof(list_t));
  char* currCommand = NULL, *sprintfBuffer = NULL;
  size_t jobCounter = 0;

  if (argc > 2) usage();
  FILE* input = (2 == argc) ? fopen(argv[1], "r") : stdin;
  if (NULL == input) {
    fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
    exit(1);
  }

  char command[512];

  if (stdin != input) shellMode = BATCH_MODE;
  else
    shellMode = INTERACTIVE_MODE;

  if (INTERACTIVE_MODE == shellMode)  write(STDOUT_FILENO, "mysh> ", 6);
  while (NULL != fgets(command, sizeof(command), input)) {
    char* token = NULL;
    currCommand = command;
    char* newLine = strchr(currCommand, '\n');
    if (NULL != newLine) *newLine = '\0';
    char* context;
    char* argvs[512];
    int i = 0;

    while (NULL != (token = strtok_r(currCommand, " ", &context))) {
      currCommand = NULL;
      argvs[i++] = token;
    }

    if (0 == strcmp(argvs[i-1], "&")) {
      jobType = BACKGROUND_JOB;
      argvs[i-1] = NULL;
      i--;
    } else {
      jobType = FOREGROUND_JOB;
      argvs[i] = NULL;
    }

    currCommand = (char*)malloc(sizeof(char)*512);
    sprintfBuffer = currCommand;
    int j = 0;
    sprintfBuffer+=snprintf(sprintfBuffer, strlen(argvs[j])+1, "%s", argvs[j]);
    j++;

    while (NULL != argvs[j]) {
      sprintfBuffer +=
      snprintf(sprintfBuffer, strlen(argvs[j])+2, " %s", argvs[j]);
      j++;
    }

    if (1 == i) {
      if (0 == strcmp("exit", argvs[0])) {
        jobType = BUILTIN_JOB;
        builtinType = EXIT_CMD;
      } else if (0 == strcmp("j", argvs[0])) {
        jobType = BUILTIN_JOB;
        builtinType = J_CMD;
      } else if (0 == strcmp("myw", argvs[0])) {
        jobType = BUILTIN_JOB;
        builtinType = MYW_CMD;
      }
    }

    if (BUILTIN_JOB != jobType) {
      int pid = fork();
      if (0 == pid) {
        if (BATCH_MODE == shellMode) {
          write(STDOUT_FILENO, currCommand, strlen(currCommand));
          write(STDOUT_FILENO, "\n", 1);
        }
        execvp(argvs[0], argvs);
        fprintf(stderr, "%s: Command not found\n", argvs[0]);
        exit(1);
      } else if (0 < pid) {
        process_t* currProcess = (process_t*)malloc(sizeof(process_t));
        currProcess->jid = ++jobCounter;
        currProcess->pid = pid;
        currProcess->status = STATUS_RUNNING;
        currProcess->command = strdup(currCommand);
        free(currCommand);
        currCommand = NULL;
        sprintfBuffer = NULL;
        listAppendNode(processList, currProcess);
        if (FOREGROUND_JOB == jobType) {
          int status = 0;
          (void) waitpid(pid, &status, 0);
          currProcess->status = STATUS_TERMINATED;
        }
      } else {
        write(STDERR_FILENO, "fork", 4);
      }
    } else {
      if (BATCH_MODE == shellMode) {
        write(STDOUT_FILENO, currCommand, strlen(currCommand));
        write(STDOUT_FILENO, "\n", 1);
        free(currCommand);
        currCommand = NULL;
        sprintfBuffer = NULL;
      }
      if (EXIT_CMD == builtinType) break;
      else if (J_CMD == builtinType) decompileList(processList);
    }

    if (INTERACTIVE_MODE == shellMode) write(STDOUT_FILENO, "mysh> ", 6);
  }
  return 0;
}