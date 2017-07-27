#ifndef COMMON_H
#define COMMON_H
#include "address_space.h"
#include "alloc.h"

struct filesystem_opts {
  size_t size;
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
