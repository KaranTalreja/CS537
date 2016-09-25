#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

typedef enum __jobType {
  FOREGROUND_JOB,
  BACKGROUND_JOB,
  BUILTIN_JOB
} jobType_e;

typedef enum __mode {
  BATCH_MODE,
  INTERACTIVE_MODE
} mode_e;

// @brief Function to print out usage of this program in case of invalid args
void usage() {
  char* usageStr = "Usage: mysh [batchFile]\n";
  write(STDERR_FILENO, usageStr, strlen(usageStr));
  exit(1);
}

int
main(int argc, char* argv[]) {
  mode_e shellMode = INTERACTIVE_MODE;
  jobType_e jobType;
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
    char* currCommand = command;
    char* newLine = strchr(currCommand, '\n');
    if (NULL != newLine) *newLine = '\0';
    char* context;
    char* argvs[512];
    int i = 0;
    while (NULL != (token = strtok_r(currCommand, " ", &context))) {
      currCommand = NULL;
      argvs[i++] = strdup(token);
    }
    if (0 == strcmp(argvs[i-1], "&")) {
      jobType = BACKGROUND_JOB;
      argvs[i-1] = NULL;
    } else {
      jobType = FOREGROUND_JOB;
      argvs[i] = NULL;
    }
    int pid = fork();
    if (0 == pid) {
      if (BATCH_MODE == shellMode) {
        int j = 0;
        while (NULL != argvs[j]) {
          write(STDOUT_FILENO, argvs[j], strlen(argvs[j]));
          write(STDOUT_FILENO, " ", 1);
          j++;
        }
        write(STDOUT_FILENO, "\n", 1);
      }
      execvp(argvs[0], argvs);
      write(STDERR_FILENO, "execvp", 6);
      exit(1);
    } else if (0 < pid) {
      if (FOREGROUND_JOB == jobType) (void) wait(NULL);
    } else {
      write(STDERR_FILENO, "fork", 4);
    }
    if (INTERACTIVE_MODE == shellMode) write(STDOUT_FILENO, "mysh> ", 6);
  }
  return 0;
}
