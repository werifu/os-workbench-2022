#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[]) {
  assert(0);
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);
  return 0;
}
