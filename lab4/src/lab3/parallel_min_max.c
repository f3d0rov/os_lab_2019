#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

#define FILE_PREFIX "/tmp/parallel_min_max_"

int get_number_length(int a);
char *get_file_path(int i);

bool g_alarm = false;
void alarmSignalHandler(int);

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int timeout = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {
                                        {"seed",          required_argument,  0, 0    },
                                        {"array_size",    required_argument,  0, 0    },
                                        {"pnum",          required_argument,  0, 0    },
                                        {"timeout",       required_argument,  0, 0    },
                                        {"by_files",      no_argument,        0, 'f'  },
                                        {0,               0,                  0, 0    }
                                    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            // error handling
            break;
          case 1:
            array_size = atoi(optarg);
            //  your code here

            if (array_size <= 0) {
              printf("array_size must be a positive integer\n");
              return -1;
            }

            // error handling
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            // error handling
            if (pnum <= 0) {
              printf("pnum must be a positive integer\n");
              return -1;
            }
            break;
          case 3:
            timeout = atoi(optarg);

            if (timeout <= 0) {
              printf("timeout must be positive\n");
              return -1;
            }

            break;

          case 4:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }



  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int start = 0, end = 0;
  int step = (array_size + pnum - 1) / (pnum); // elements / process
  signal(SIGCHLD, SIG_IGN); // Ignore children return

  int (*pipefds)[2];
  if (!with_files) {
    pipefds = malloc(sizeof(int[2]) * pnum);
    for (int i = 0; i < pnum; ++i) {
      int e = pipe(pipefds[i]);
      if (e < 0) {
        switch (errno) {
          case EFAULT:
            printf("pipe() error (EFAULT): pipefd is not valid\n");
            break;
          case EMFILE:
            printf("pipe() error (EMFILE): the per-process limit on the number of open file descriptors has been reached\n");
            break;
          case ENFILE:
            printf("pipe() error (ENFILE): the user hard limit on memory that can be allocated for pipes has been reached and the caller is not privileged");
            break;
          default:
            printf("pipe() error #%d\n", errno);
        }
        
        return -1;
      }
    }
  }
  
  signal(SIGALRM, alarmSignalHandler);

  if (timeout > 0) alarm(timeout);

  for (int i = 0; i < pnum; i++) {
    
    pid_t child_pid = fork();

    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process

        // parallel somehow

        start = i * step;
        end = i * (step + 1);
        if (end > array_size) end = array_size;

        struct MinMax mm = GetMinMax(array, start, end);

        if (with_files) {
          // use files here
          
          char *path = get_file_path(i);
          FILE *f = fopen(path, "w");
          fprintf(f, "%d %d", mm.min, mm.max);
          fclose(f);

          free(path);

        } else {
          // use pipe here
          write(pipefds[i][1], &mm.min, sizeof(mm.min));
          write(pipefds[i][1], &mm.max, sizeof(mm.max));
          close(pipefds[i][0]);
          close(pipefds[i][1]);
        }

        return 0;
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    // your code here
    int waitStatus = waitpid(-1, NULL, WNOHANG);
    if (waitStatus > 0) {
      active_child_processes -= 1;
      printf(".%d\n", active_child_processes);
    } else if (waitStatus != 0) {
      if (errno == ECHILD) { // No children
        active_child_processes = 0;
      } else {
        printf("wait(NULL) returned error %d\n", errno);
        return -1;
      }
    }

    if (g_alarm) {
      printf("Timed out after %d seconds, stopping execution\n", timeout);
      signal(SIGQUIT, SIG_IGN);
      kill(0, SIGQUIT);
      return 0;
    }
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char *path = get_file_path(i);
      FILE *f = fopen(path, "r");
      fscanf(f, "%d %d", &min, &max);
      fclose(f);

      free(path);
    } else {
      // Pipes
      read(pipefds[i][0], &min, sizeof(min));
      read(pipefds[i][0], &max, sizeof(max));
      close(pipefds[i][0]);
      close(pipefds[i][1]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  if (!with_files) {
    free(pipefds);
  }

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}


int get_number_length(int a) {
  if (a == 0) return 1;
  int result = a < 0 ? 1 : 0;
  
  while (a > 0) {
    a /= 10;
    ++result;
  }

  return result;
}

char *get_file_path(int i) {
  char *number = 0;

  if (i) {
    number = (char *) malloc(sizeof(char) * (get_number_length(i) + 1));
    sprintf(number, "%d", i);
  } else {
    number = (char *) malloc(sizeof(char) * 2);
    number[0] = '0';
    number[1] = 0;
  }
  char *path = malloc(sizeof(char) * (strlen(FILE_PREFIX) + get_number_length(i) + 1));
  strcpy(path, FILE_PREFIX);
  strcat(path, number);

  free(number);
  return path;
}

void alarmSignalHandler(int signum) {
  g_alarm = true;
}
