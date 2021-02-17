#include "inode.h"

#include <cassert>
#include <fuse.h>
#include <string.h>

#include "filesystem.h"

Inode::~Inode() {}

/*
 * FIXME: space should be freed here, but also when it is deleted, if there
 * are no other open file handles. Otherwise, space is only freed after the
 * file is deleted and the kernel releases its references.
 */
RegInode::~RegInode() {
    for (auto it = extents_.begin(); it != extents_.end(); it++)
        fs_->free_space(&it->second);
    extents_.clear();
}

bool Inode::is_regular() const { return i_st.st_mode & S_IFREG; }

bool Inode::is_directory() const { return i_st.st_mode & S_IFDIR; }

bool Inode::is_symlink() const { return i_st.st_mode & S_IFLNK; }
