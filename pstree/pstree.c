#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
// bool true & false
#define true 1
#define false 0

// unix succ & fail
#define succ 0
#define fail -1

typedef char bool;

bool opt_v = false;
bool opt_n = false;
bool opt_p = false;

bool is_opt_p(char* arg, int len) {
  if (len != 2 && len != 11) return false;
  if (len == 2) {
    char str[] = "-p";
    for (int i = 0; i < len; i++) {
      if (arg[i] != str[i]) return false;
    }
    return true;
  }
  assert(len == 11);
  char str[] = "--show-pids";
  for (int i = 0; i < len; i++) {
    if (arg[i] != str[i]) return false;
  }
  return true;
}

bool is_opt_n(char* arg, int len) {
  if (len != 2 && len != 14) return false;
  if (len == 2) {
    char str[] = "-n";
    for (int i = 0; i < len; i++) {
      if (arg[i] != str[i]) return false;
    }
    return true;
  }
  assert(len == 14);
  char str[] = "--numeric-sort";
  for (int i = 0; i < len; i++) {
    if (arg[i] != str[i]) return false;
  }
  return true;
}

bool is_opt_v(char* arg, int len) {
  if (len != 2 && len != 9) return false;
  if (len == 2) {
    char str[] = "-v";
    for (int i = 0; i < len; i++) {
      if (arg[i] != str[i]) return false;
    }
    return true;
  }
  assert(len == 9);
  char str[] = "--version";
  for (int i = 0; i < len; i++) {
    if (arg[i] != str[i]) return false;
  }
  return true;
}

void parse_opt(int argc, char *argv[]) {
  if (argc <= 1) return;
  for (int i = 1; i < argc; i++) {
    int len = strlen(argv[i]);
    if (is_opt_v(argv[i], len)) {
      opt_v = true;
      // no need to analyse other options
      return;
    }
    if (is_opt_n(argv[i], len)) {
      opt_n = true;
      continue;
    }
    if (is_opt_p(argv[i], len)) {
      opt_p = true;
      continue;
    }
  }
  return;
}

// returns -1 if not number
int parse_digits(char* str) {
  int len = strlen(str);
  int result = 0;
  for (int i = 0; i < len; i++) {
    if (str[i] < '0' || str[i] > '9') return -1;
    int digit = str[i] - '0';
    result *= 10;
    result += digit;
  }
  return result;
}

int pids[4096];
// returns the number of processes 
int load_proc() {
  struct dirent *pDirent;
  DIR *pDir;
  pDir = opendir("/proc/");
  if (pDir == NULL) {
      printf ("Cannot open directory '/proc/'\n");
      return 1;
  }
  int idx = 0;
  while ((pDirent = readdir(pDir)) != NULL) {
      int num = parse_digits(pDirent->d_name);
      if (num == -1) continue;
      printf("[%d]\n", num);
      pids[idx++] = num;
  }
  return idx;
}

int main(int argc, char *argv[]) {
  parse_opt(argc, argv);
  if (opt_v) {
    printf("version: 0.0.1\n");
    return 0;
  }
  if (opt_n) {
    printf("opt_n\n");
  }
  if (opt_p) {
    printf("opt_p\n");
  }
  load_proc();
  return 0;
}
