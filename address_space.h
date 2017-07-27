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

class Node {
 public:
  Node(void *base, uintptr_t size) :
    base_((char*)base), size_(size)
  {}

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

class AddressSpace {
 public:
  int init(struct filesystem_opts *opts) {
    const size_t size = opts->size;

    void *data = mmap(NULL, size, PROT_READ|PROT_WRITE,
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if (data == MAP_FAILED)
      return -ENOMEM;

    auto node = new Node(data, size);

    nodes_.push_back(node);

    return 0;
  }

  std::vector<Node*>& nodes() {
    return nodes_;
  }

 private:
  std::vector<Node*> nodes_;
};

struct NodeAlloc {
  NodeAlloc(Node *n) :
    node(n), alloc(new Allocator(n->size()))
  {}

  Node *node;
  Allocator *alloc;
};

struct Extent {
  // logical
  size_t length;

  // physical
  NodeAlloc *node;
  size_t addr;
  size_t size;
};

#endif
