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
#define SEED "0"
#define ARRAY_SIZE "10"

int main() {
  int f = fork();
  if (f > 0) {
    execl(EXEC_PATH, EXEC_PATH, SEED, ARRAY_SIZE, NULL);
  }
  return 0;
}
