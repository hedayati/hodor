#include <sys/mman.h>

#include <hodor-plib.h>
#include <hodor.h>

#include <cstring>
#include <iostream>

#include <boost/container/map.hpp>
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/map.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

#include "kv.h"

#define HEAP_SIZE 4096 * PAGE_SIZE

boost::container::pmr::monotonic_buffer_resource *pool;
boost::container::pmr::map<std::string, std::string> *map;

int kv_init(void) {
  /*
   * Here we allocate the private heap for map. However, note that this
   * is __not__ completely correct/safe since both pool and map objects are
   * allocated from the shared heap -- but you get the idea.
   */
  void *heap_pages = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  pool = new boost::container::pmr::monotonic_buffer_resource{
      heap_pages, HEAP_SIZE, boost::container::pmr::null_memory_resource()};

  /* Instruct map to use the private heap for allocations. */
  map = new boost::container::pmr::map<std::string, std::string>{pool};

  /*
   * Mark the private heap with PKEY = 1. We do it at the end since kv_init() is
   * called during load with PKRU = 0x55555554.
   */
  pkey_mprotect(heap_pages, HEAP_SIZE, PROT_READ | PROT_WRITE, 1);
  return 0;
}
HODOR_INIT_FUNC(kv_init);

HODOR_FUNC_ATTR bool kv_get(const std::string &key, std::string &value) {
  /*
   * TODO: do copy_in/_out and check the arguments.
   * Let's fault (e.g., SIGSEGV) here before we start accessing private data.
   * There should be no unsafe access beyond this point:
   * ------------------------------------------------------------------------
   */
  if (map->find(key) == map->end()) return false;
  value = map->find(key)->second;
  return true;
}
HODOR_FUNC_EXPORT(kv_get, 2);

HODOR_FUNC_ATTR void kv_put(const std::string &key, const std::string &value) {
  /*
   * TODO: do copy_in/_out and check the arguments.
   * Let's fault (e.g., SIGSEGV) here before we start accessing private data.
   * There should be no unsafe access beyond this point:
   * ------------------------------------------------------------------------
   */
  map->emplace(key, value);
}
HODOR_FUNC_EXPORT(kv_put, 2);
