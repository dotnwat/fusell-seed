#include "inode_index.h"
#include <cassert>

Inode::Ptr InodeIndex::inode(fuse_ino_t ino)
{
  return refs_.at(ino).second;
}

DirInode::Ptr InodeIndex::dir_inode(fuse_ino_t ino)
{
  auto in = inode(ino);
  assert(in->is_directory());
  return std::static_pointer_cast<DirInode>(in);
}

SymlinkInode::Ptr InodeIndex::symlink_inode(fuse_ino_t ino)
{
  auto in = inode(ino);
  assert(in->is_symlink());
  return std::static_pointer_cast<SymlinkInode>(in);
}

uint64_t InodeIndex::nfiles()
{
  uint64_t ret = 0;
  for (auto it = refs_.begin(); it != refs_.end(); it++)
    if (it->second.second->i_st.st_mode & S_IFREG)
      ret++;
  return ret;
}
