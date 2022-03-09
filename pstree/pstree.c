#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

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
  // pids[0] is 0
  int idx = 1;
  // maybe asc order
  while ((pDirent = readdir(pDir)) != NULL) {
      int num = parse_digits(pDirent->d_name);
      if (num == -1) continue;
      // printf("[%d]\n", num);
      pids[idx++] = num;
  }
  assert(pids[idx] == 0 && pids[idx-1] > 0);
  return idx;
}

typedef struct ProcNode {
  int pid;
  char exec_name[256];
  int  name_size;
  int ppid;

  // children[i] is its i-th child
  struct ProcNode* children[256];
  int child_num;
} ProcNode;
ProcNode* nodes[MAX_PROC_NUM];

// init nodes[idx]
void parse_node(char* buf, int len, int idx) {
  nodes[idx] = (ProcNode*)malloc(sizeof(ProcNode));
  ProcNode* node = nodes[idx];
  int i = 0;
  // cat /proc/[pid]/stat 
  // => pid comm state ppid ...

  // pid
  int pid = 0;
  while (buf[i] != ' ') {
    int digit = buf[i] - '0';
    pid *= 10;
    pid += digit;
    i++;
  }
  node->pid = pid;

  // comm
  i += 2;  // skip ' ' and '('
  int j = 0;
  while (buf[i] != ' ') {
    node->exec_name[j++] = buf[i];
    i++;
  }
  // eat a ')'
  node->exec_name[j-1] = 0;

  node->name_size = j;

  // state
  i += 3; // ' ' and 'S'(state) and ' '

  // ppid
  int ppid = 0;
  while (buf[i] != ' ') {
    int digit = buf[i] - '0';
    ppid *= 10;
    ppid += digit;
    i++;
  }
  node->ppid = ppid;

  // init child_num
  node->child_num = 0;
  return;
}

// returns -1 when fail occurs (such as open fail)
// returns 0 when success
int build_tree(int nodes_num) {
  nodes[0] = (ProcNode*)malloc(sizeof(ProcNode));
  nodes[0]->exec_name[0] = 0;

  // i starts at 1 because 0 is not a process
  for (int i = 1; i < nodes_num; i++) {
    int pid = pids[i];
    char path[256] = {0};
    sprintf(path, "/proc/%d/stat", pid);
    FILE* fp = fopen(path, "r");
    if (!fp) {
      printf("fail to open file %s, %d\n", path, i);
      return -1;
    }
    assert(fp);
    char stat_buf[1024];
    // n bytes are read
    int n = fread(stat_buf, 1024, 1, fp);
    
    parse_node(stat_buf, n, i);
  }

  // nodes[0] is pid=0
  for (int i = 1; i < nodes_num; i++) {
    ProcNode* node = nodes[i];
    // fill the children attribute of node's parent
    int j = 0;
    while (j <= i) {
      if (nodes[j]->pid == node->ppid) {
        ProcNode* parent = nodes[j];
        parent->children[parent->child_num++] = node;
        // printf("nodes[%d]->ppid == %d, nodes[%d]->pid == %d\n", i, node->ppid, j, nodes[j]->pid);
        break;
      }
      j++;
    }
    assert(j < i + 1);  // must find

  }
  return 0;
}

void print_nodes_arr(int proc_num) {
  for (int i = 0; i < proc_num; i++) {
    ProcNode* node = nodes[i];
    printf("[pid:%d,\texec_name:%s,\tppid:%d,\t", node->pid, node->exec_name, node->ppid);
    printf("children-pid: ");
    for (int j = 0; j < node->child_num; j++) {
      printf("%d ", node->children[j]->pid);
    }
    printf("\n");
  }
}

void print_pids(int num) {
  for (int i = 0; i < num; i++) {
    printf("%d ", pids[i]);
  }
  printf("\n");
}

inline int max(int a, int b) {return a > b? a: b;}

int max_row = 0;
char tree_str[512][256] = {0};

// load tree into string
void load_tree(ProcNode* root, int row, int col) {
  if (!root) return;
  tree_str[row][col++] = '-';
  for (int i = 0; i < root->name_size; i++) {
    tree_str[row][col++] = root->exec_name[i];
  }
  printf("%s\n", root->exec_name);
  // now '-process_name' is loaded
  max_row = max(max_row, row);
  if (root->child_num <= 0) {
    return;
  }
  tree_str[row][col++] = '-';
  // '-+-aaa
  //   |
  //   +-aa'
  //   ^ this one
  if (root->child_num == 1) {
    tree_str[row][col++] = '-';
    load_tree(root->children[0], row, col);
  } else {
    assert(root->child_num >= 2);

    // first node '+-xxx'
    tree_str[row][col++] = '+';
    load_tree(root->children[0], row, col);
    // to next child
    row++;

    for (int i = 1; i < root->child_num-1; i++) {
      while (row <= max_row) {
        for (int j = 0; j < row; j++) tree_str[row][j] = ' ';
        tree_str[row++][col] = '|';
      }
      tree_str[row][col++] = '+';
      load_tree(root->children[i], row, col);
      row++;
    }

    // last node '   `-xxx'
    while (row <= max_row) {
      for (int j = 0; j < row; j++) tree_str[row][j] = ' ';
      tree_str[row++][col] = '|';
    }
    tree_str[row][col++] = '`';
    load_tree(root->children[0], row, col);
  }
}

void print_tree() {
  for (int i = 0; i <= max_row; i++) {
    printf("%s\n", tree_str[i]);
  }
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
  int proc_num = load_proc();
  // print_pids(proc_num);
  build_tree(proc_num); // add a pid=0 node
  // print_nodes_arr(proc_num);
  load_tree(nodes[0], 0, 0);
  print_tree();
  return 0;
}
