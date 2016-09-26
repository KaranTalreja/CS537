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

typedef enum __builtin {
  EXIT_CMD,
  J_CMD,
  MYW_CMD
} builtin_e;

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
  builtin_e builtinType;
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
      i--;
    } else {
      jobType = FOREGROUND_JOB;
      argvs[i] = NULL;
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
          int j = 0;
          while (NULL != argvs[j]) {
            write(STDOUT_FILENO, argvs[j], strlen(argvs[j]));
            write(STDOUT_FILENO, " ", 1);
            j++;
          }
          write(STDOUT_FILENO, "\n", 1);
        }
        execvp(argvs[0], argvs);
        fprintf(stderr, "%s: Command not found\n", argvs[0]);
        exit(1);
      } else if (0 < pid) {
        if (FOREGROUND_JOB == jobType) (void) wait(NULL);
      } else {
        write(STDERR_FILENO, "fork", 4);
      }
    } else {
      if (BATCH_MODE == shellMode) {
        write(STDOUT_FILENO, argvs[0], strlen(argvs[0]));
        write(STDOUT_FILENO, "\n", 1);
      }
      if (EXIT_CMD == builtinType) break;
    }
    if (INTERACTIVE_MODE == shellMode) write(STDOUT_FILENO, "mysh> ", 6);
  }
  return 0;
}
