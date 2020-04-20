#include <assert.h>
#include <hodor.h>
#include <kv.h>
#include <stdio.h>

#include <iostream>
#include <string>

int main(int argc, char const *argv[]) {
  int ret;
  std::string value;

  ret = hodor_init();
  assert(ret == 0);
  ret = hodor_enter();
  assert(ret == 0);

  for (int i = 0; i < 10000; i++) {
    kv_put("abc" + std::to_string(i), std::to_string(i) + "def");
  }

  if (kv_get("abc134", value))
    std::cout << "value: " << value << std::endl;
  else
    std::cout << "value not found." << std::endl;
}

