#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#define EXEC_PATH "./sequential_min_max"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s seed arraysize\n", argv[0]);
    return 1;
  }

  int f = fork();
  if (f > 0) {
    printf("child PID: %d\n", f);
    wait(NULL);
  } else {
    execv(EXEC_PATH, argv);
  }
  return 0;
}
