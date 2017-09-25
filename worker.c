#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int factorial(int n) { return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n; }

int main(int argc, char **argv) {
  int x = 0;
  int n = 0;
  int flagx = 0;
  int flagn = 0;
  opterr = 0;
  long factorial_n = 1;
  double pow_x = 0;
  double ans = 0;
  int c;
  while ((c = getopt(argc, argv, "x:n:")) != -1)
    switch (c) {
    case 'x':
      x = atoi(optarg);
      flagx = 1;
      break;
    case 'n':
      n = atoi(optarg);
      flagn = 1;
      break;
    case '?':
      if (optopt == 'x' || optopt == 'n') {
        fprintf(stderr, "-%c needs a parameter\n", optopt);
      } else if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      break;
    default:
      abort();
    }
  if (flagx == 0 || flagn == 0) {
    fprintf(stderr, "usage: %s -x integer_value -n integer_value\n", argv[0]);
    exit(1);
  }
  factorial_n = factorial(n);
  pow_x = pow(x, n);
  ans = pow_x / factorial_n;
  int fd = STDOUT_FILENO;
  struct stat statbuf;
  fstat(fd, &statbuf);
  if (S_ISFIFO(statbuf.st_mode)) {
    printf("%lf", ans);
  } else
    printf("x^n / n! : %lf\n", ans);
  return 0;
}
