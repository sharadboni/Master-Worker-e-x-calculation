#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(int argc, char **argv) {

  int flaga = 0;
  int flagb = 0;
  int flagc = 0;
  int flagx = 0;
  int flagn = 0;
  char *x;
  int n = 0;
  int n_worker = 0;
  char *w_path;
  char *mechanism;
  int c;
  while (1) {
    static struct option long_options[] = {
        {"worker_path", required_argument, 0, 'a'},
        {"num_workers", required_argument, 0, 'b'},
        {"wait_mechanism", required_argument, 0, 'c'},
        {0, 0, 0, 0}};
   
    int option_index = 0;

    c = getopt_long(argc, argv, "a:b:c:x:n:", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {

    case 'a':
      w_path = optarg;
      flaga = 1;
      break;

    case 'b':
      n_worker = atoi(optarg);
      flagb = 1;
      break;

    case 'c':
      mechanism = optarg;
      flagc = 1;
      break;

    case 'x':
      x = optarg;
      flagx = 1;
      break;

    case 'n':
      n = atoi(optarg);
      flagn = 1;
      break;

    case '?':
      fprintf(stderr, "wrong argument usage");
      break;

    default:
      abort();
    }
  }
  if (flagx == 0 || flagn == 0 || flaga == 0 || flagb == 0 || flagc == 0) {
    fprintf(stderr, "usage: %s --worker_path ./worker --num_workers 5 "
                    "--wait_mechanism MECHANISM -x 2 -n 12\n",
            argv[0]);
    exit(1);
  }
  double sum = 0;
  char buffer[100];
  pid_t pid;
  int readfd[n];
  int p[n][2];
  for (int i = 0; i <= n; i++) {
    pipe(p[i]);
    readfd[i] = 0;
  }
  int k = 0;
  int count = 0;
  fd_set set;
  int total_count = 0;

  struct epoll_event event, events[n];
  int epfd = epoll_create(n + 1);

  if (mechanism == "epoll") {
    while (k <= n) {

      if (k <= n && count <= n_worker) {
        pid = fork();
        count += 1;
        if (pid == 0) {
          dup2(p[k][1], STDOUT_FILENO);
          close(p[k][0]);
          char n_buf[20];
          sprintf(n_buf, "%d", k);
          execl(w_path, w_path, "-x", x, "-n", n_buf, (char *)0);
        } else {
          close(p[k][1]);
          event.events = EPOLLIN;
          event.data.fd = p[k][0];
          epoll_ctl(epfd, EPOLL_CTL_ADD, p[k][0], &event);
        }
        k += 1;
      } else {
        for (int i = 0; i < epoll_wait(epfd, events, n + 1, -1); i++) {
          for (int j = 0; j <= k; j++) {
            if (events[i].data.fd == p[j][0] && readfd[j] == 0) {
              count -= 1;
              total_count += 1;
              readfd[j] = 1;
              int ind = 0;
              char c;
              while (read(p[j][0], &c, 1) == 1) {
                buffer[ind] = c;
                ind++;
              }
              buffer[ind] = '\0';
              sum += atof(buffer);
              printf("worker %d: %s^%d / %d! : %s\n", j, x, j, j, buffer);
              close(p[j][0]);
            }
          }
        }
      }
    }

    while (total_count <= n) {
      for (int i = 0; i < epoll_wait(epfd, events, n + 1, -1); i++) {
        for (int j = 0; j <= n; j++) {
          if (events[i].data.fd == p[j][0] && readfd[j] == 0) {
            total_count += 1;
            readfd[j] = 1;
            int ind = 0;
            char c;
            while (read(p[j][0], &c, 1) == 1) {
              buffer[ind] = c;
              ind++;
            }
            buffer[ind] = '\0';
            sum += atof(buffer);
            printf("worker %d: %s^%d / %d! : %s\n", j, x, j, j, buffer);
            close(p[j][0]);
          }
        }
      }
    }

  } else {
    while (k <= n) {
      FD_ZERO(&set);
      for (int i = 0; i <= k; i++) {
        if (readfd[i] == 0) {
          FD_SET(p[i][0], &set);
        }
      }

      if (k <= n && count <= n_worker) {
        pid = fork();
        count += 1;
        if (pid == 0) {
          dup2(p[k][1], STDOUT_FILENO);
          close(p[k][0]);
          char n_buf[20];
          sprintf(n_buf, "%d", k);
          execl(w_path, w_path, "-x", x, "-n", n_buf, (char *)0);
        } else {
          close(p[k][1]);
          FD_SET(p[k][0], &set);
        }
        k += 1;
      } else {
        select(FD_SETSIZE, &set, NULL, NULL, NULL);
        for (int i = 0; i <= k; i++) {
          if (FD_ISSET(p[i][0], &set)) {
            count -= 1;
            total_count += 1;
            readfd[i] = 1;
            int ind = 0;
            char c;
            while (read(p[i][0], &c, 1) == 1) {
              buffer[ind] = c;
              ind++;
            }
            buffer[ind] = '\0';
            sum += atof(buffer);
            printf("worker %d: %s^%d / %d! : %s\n", i, x, i, i, buffer);
            close(p[i][0]);
          }
        }
      }
    }

    while (total_count <= n) {
      FD_ZERO(&set);
      for (int i = 0; i <= n; i++) {
        if (readfd[i] == 0) {
          FD_SET(p[i][0], &set);
        }
      }
      select(FD_SETSIZE, &set, NULL, NULL, NULL);
      for (int i = 0; i <= n; i++) {
        if (FD_ISSET(p[i][0], &set)) {
          count -= 1;
          total_count += 1;
          readfd[i] = 1;
          int ind = 0;
          char c;
          while (read(p[i][0], &c, 1) == 1) {
            buffer[ind] = c;
            ind++;
          }
          buffer[ind] = '\0';
          sum += atof(buffer);
          printf("worker %d: %s^%d / %d! : %s\n", i, x, i, i, buffer);
          close(p[i][0]);
        }
      }
    }
  }
  printf("e^x : %lf\n", sum);
  return 0;
}
