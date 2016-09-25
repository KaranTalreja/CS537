#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

int
main(int argc, char* argv[]) {
  char command[512];
  printf("mysh> ");
  while (NULL != fgets(command, sizeof(command), stdin)) {
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
    bool runInBackground = false;
    if (0 == strcmp(argvs[i-1], "&")) {
      runInBackground = true;
      argvs[i-1] = NULL;
    } else {
      argvs[i] = NULL;
    }
    int pid = fork();
    if (0 == pid) {
      execvp(argvs[0], argvs);
      perror("execvp");
      exit(1);
    } else if (0 < pid) {
      if (false == runInBackground) (void) wait(NULL);
    } else {
      perror("fork");
    }
    printf("mysh> ");
  }
  return 0;
}
