#include "address_space.h"
#include <cassert>
#include <sys/mman.h>
#include <iostream>
#include <string.h>
#include "common.h"

class LocalNodeImpl : public Node {
 public:
  LocalNodeImpl(void *base, uintptr_t size) :
    base_((char*)base), size_(size)
  {
#if 0
    std::cout << "local mem: base=" << base
      << " size=" << size << std::endl;
#endif
  }

  size_t size() {
    return size_;
  }

  void read(void *dst, void *src, size_t len) {
    char *abs_src = base_ + (uintptr_t)src;
    assert((abs_src + len - 1) < (base_ + size_));
    memcpy(dst, abs_src, len);
  }

  void write(void *dst, void *src, size_t len) {
    char *abs_dst = base_ + (uintptr_t)dst;
    assert((abs_dst + len - 1) < (base_ + size_));
    memcpy(abs_dst, src, len);
  }

 private:
  char *base_;
  uintptr_t size_;
};

int LocalAddressSpace::init(struct filesystem_opts *opts)
{
  const size_t size = opts->size;

  void *data = mmap(NULL, size, PROT_READ|PROT_WRITE,
      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

  if (data == MAP_FAILED)
    return -ENOMEM;

  LocalNodeImpl *node = new LocalNodeImpl(data, size);

  nodes_.push_back(node);

  return 0;
}
