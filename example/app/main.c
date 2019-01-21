#include <hodor.h>
#include <stdio.h>

#include <plib.h>

int main(int argc, char const *argv[]) {
  int ret = 0;

  ret = hodor_init();
  ret = hodor_enter();

  printf("hello world! %d\n", plib_sum(6, 7));

  return 0;
}