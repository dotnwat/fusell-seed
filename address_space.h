#ifndef GASSYFS_ADDRESS_SPACE_H_
#define GASSYFS_ADDRESS_SPACE_H_
#include <cstddef>
#include <vector>
#include <sys/mman.h>
#include <cstring>
#include "alloc.h"

struct filesystem_opts {
  size_t size;
};

class AddressSpace {
 public:
  int init(struct filesystem_opts *opts) {
    const size_t size = opts->size;

    void *data = mmap(NULL, size, PROT_READ|PROT_WRITE,
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if (data == MAP_FAILED)
      return -ENOMEM;

    base_ = (char*)data;
    size_ = size;

    return 0;
  }

  size_t size() const {
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

struct Extent {
  // logical
  size_t length;

  // physical
  size_t addr;
  size_t size;
};

#endif
