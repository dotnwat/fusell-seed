#pragma once
#include <unordered_map>
#include <cassert>
#include <fuse.h>
#include "inode.h"

/*
 * This container maps inode number to inode pointer. Each inode has an
 * associated reference count, and when the reference count falls to zero that
 * inode is removed. The reference count corresponds to the number of
 * outstanding references that the kernel inode cache has for an inode.
 */
class InodeIndex {
 public:
  /*
   * insert an inode into the index. this is the same as `get` but asserts
   * that the inode is not in the index. reference counts start at one.
   */
  void add(const std::shared_ptr<Inode>& inode) {
    auto ret = refs_.emplace(inode->ino, std::make_pair(1, inode));
    assert(ret.second);
  }

  /*
   * increase the reference count on an inode. if the inode isn't present in
   * the index it is added with an initial count of one.
   */
  void get(const std::shared_ptr<Inode>& inode) {
    auto ret = refs_.emplace(inode->ino, std::make_pair(1, inode));
    if (!ret.second) {
      assert(ret.first->second.first > 0);
      ret.first->second.first++;
    }
  }

  // decrease the reference count by given amount.
  void put(fuse_ino_t ino, long int dec) {
    auto it = refs_.find(ino);
    assert(it != refs_.end());
    assert(it->second.first > 0);
    it->second.first -= dec;
    assert(it->second.first >= 0);
    if (it->second.first == 0) {
      refs_.erase(it);
    }
  }

  // lookup inodes by type
  Inode::Ptr inode(fuse_ino_t ino);
  DirInode::Ptr dir_inode(fuse_ino_t ino);
  SymlinkInode::Ptr symlink_inode(fuse_ino_t ino);

  uint64_t nfiles();

 private:
  std::unordered_map<fuse_ino_t,
    std::pair<long int, Inode::Ptr>> refs_;
};
