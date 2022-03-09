#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
// bool true & false
#define true 1
#define false 0
typedef char bool;

// unix succ & fail
#define succ 0
#define fail -1

#define MAX_PROC_NUM 4096

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

int pids[MAX_PROC_NUM] = {0};
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
      // printf("[%d]\n", num);
      pids[idx++] = num;
  }
  assert(pids[idx] == 0 && pids[idx-1] > 0);
  return idx;
}

struct ProcNode {
  int pid;
  char* exec_name;
  int name_size;
  int ppid;

  // nodes[children[i]] is its i-th child
  int* children;
  int child_num;
};
struct ProcNode nodes[MAX_PROC_NUM];

// returns the index of the root node (pid=1)
// returns -1 when fail occurs (such as open fail)
int build_tree(int nodes_num) {
  for (int i = 0; i < nodes_num; i++) {
    int pid = pids[i];
    char path[256] = {0};
    sprintf(path, "/proc/%d/stat", pid);
    FILE* fp = fopen(path, "r");
    if (!fp) {
      printf("fail to open file %s\n", path);
      return -1;
    }
    assert(fp);
    char stat_buf[1024];
    // n bytes are read
    int n = fread(stat_buf, 1024, 1, fp);
    nodes[i] = parse_node(stat_buf);
  }

  // find the root index
  int rooti = -1;
  for (int i = 0; i < nodes_num; i++) {
    if (nodes[i].pid == 1) {
      rooti = i;
      break;
    }
  }
  assert(rooti != -1);

  // fill children attr
  return rooti;
}

(struct ProcNode) parse_node(char* buf) {
  struct ProcNode node;

  return node;
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
