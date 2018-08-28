#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <fuse.h>
#include <fuse_lowlevel.h>

class FileSystem;

struct Extent {
  Extent(size_t size) :
    size(size),
    buf(new char[size])
  {}

  size_t size;
  std::unique_ptr<char[]> buf;
};

class Inode {
 public:
  typedef std::shared_ptr<Inode> Ptr;

  Inode(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid, blksize_t blksize,
      mode_t mode, FileSystem *fs);
  virtual ~Inode() {}

  const fuse_ino_t ino;

  struct stat i_st;

  bool is_regular() const;
  bool is_directory() const;
  bool is_symlink() const;

 protected:
  FileSystem *fs_;
};

class RegInode : public Inode {
 public:
  typedef std::shared_ptr<RegInode> Ptr;
  RegInode(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid, blksize_t blksize,
      mode_t mode, FileSystem *fs) :
    Inode(ino, time, uid, gid, blksize, mode, fs) {
      i_st.st_mode = S_IFREG | mode;
    }
  ~RegInode();
  std::map<off_t, Extent> extents_;
};

class DirInode : public Inode {
 public:
  typedef std::shared_ptr<DirInode> Ptr;
  typedef std::map<std::string, Inode::Ptr> dir_t;
  DirInode(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid, blksize_t blksize,
      mode_t mode, FileSystem *fs) :
    Inode(ino, time, uid, gid, blksize, mode, fs) {
      i_st.st_nlink = 2;
      i_st.st_mode = S_IFDIR | mode;
      i_st.st_blocks = 1;
    }
  dir_t dentries;
};

class SymlinkInode : public Inode {
 public:
  typedef std::shared_ptr<SymlinkInode> Ptr;
  SymlinkInode(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid, blksize_t blksize,
      const std::string& link, FileSystem *fs) :
    Inode(ino, time, uid, gid, blksize, 0, fs) {
      i_st.st_mode = S_IFLNK;
      this->link = link;
      i_st.st_size = link.length();
    }
  std::string link;
};
