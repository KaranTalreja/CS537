#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<errno.h>
#include<sys/time.h>

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
outputRunningJobs(list_t* inList) {
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
getJob(list_t* inList, unsigned int jid) {
  if ((NULL == inList) || (NULL == inList->first)) return 0;
  listnode_t *current = inList->first;
  while (NULL != current) {
    listnode_t* currRecord = current;
    if (currRecord->info->jid == jid) return currRecord->info->pid;
    current = current->next;
  }
  return -1;
}

int
main(int argc, char* argv[]) {
  mode_e shellMode = INTERACTIVE_MODE;
  jobType_e jobType;
  builtin_e builtinType;
  list_t* processList = (list_t*)malloc(sizeof(list_t));
  processList->first = NULL;
  processList->current = NULL;
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
    if (0 == i) continue;  // Empty command
    if (0 == strcmp(argvs[i-1], "&")) {
      jobType = BACKGROUND_JOB;
      argvs[i-1] = NULL;
      i--;
    } else if (argvs[i-1][strlen(argvs[i-1])-1] == '&') {
      jobType = BACKGROUND_JOB;
      argvs[i-1][strlen(argvs[i-1])-1] = '\0';
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
      }
    } else if (2 == i) {
      if (0 == strcmp("myw", argvs[0])) {
        jobType = BUILTIN_JOB;
        builtinType = MYW_CMD;
      }
    }
    if (BATCH_MODE == shellMode) {
      fprintf(stdout, "%s\n", currCommand);
      fflush(stdout);
    }
    if (BUILTIN_JOB != jobType) {
      int pid = fork();
      if (0 == pid) {
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
      if (EXIT_CMD == builtinType) {
        break;
      } else if (J_CMD == builtinType) {
        outputRunningJobs(processList);
      } else if (MYW_CMD == builtinType) {
        int requiredJid = atoi(argvs[1]);
        int rc = getJob(processList, requiredJid);
        if (-1 == rc) {
          fprintf(stdout, "Invalid jid %d\n", requiredJid);
          fflush(stdout);
        } else if (0 < rc) {
          int status;
          int requiredPid = waitpid(rc, &status, WNOHANG);
          if (0 < requiredPid) {
          } else if (0 == requiredPid) {
            struct timeval start;
            gettimeofday(&start, NULL);
            (void) waitpid(rc, &status, 0);
            struct timeval end;
            gettimeofday(&end, NULL);
            size_t microSecs = (end.tv_sec-start.tv_sec)*1000000
            + end.tv_usec-start.tv_usec;
            fprintf(stdout, "%ld : Job %d terminated\n",
            microSecs, requiredJid);
            fflush(stdout);
          } else if (ECHILD == errno) {
            fprintf(stdout, "0 : Job %d terminated\n", requiredJid);
            fflush(stdout);
          }
        }
      }
      free(currCommand);
      currCommand = NULL;
      sprintfBuffer = NULL;
    }
    if (INTERACTIVE_MODE == shellMode) write(STDOUT_FILENO, "mysh> ", 6);
  }
  if ((NULL == processList) || (NULL == processList->first)) return 0;
  listnode_t *current = processList->first;
  while (NULL != current) {
    listnode_t* next = current->next;
    free(current->info->command);
    free(current->info);
    free(current);
    current = next;
  }
  if (shellMode == BATCH_MODE) fclose(input);
  free(processList);
  return 0;
}
