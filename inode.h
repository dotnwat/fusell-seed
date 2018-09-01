#pragma once
#include <map>
#include <memory>
#include <string>
#include <cstring>
#include <cassert>
#include <vector>
#include <iostream>
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
  Inode(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid,
      blksize_t blksize, mode_t mode, FileSystem *fs) :
    ino(ino), fs_(fs)
  {
    memset(&i_st, 0, sizeof(i_st));
    i_st.st_ino = ino;
    i_st.st_atime = time;
    i_st.st_mtime = time;
    i_st.st_ctime = time;
    i_st.st_uid = uid;
    i_st.st_gid = gid;
    i_st.st_blksize = blksize;
  }

  virtual ~Inode() = 0;

  const fuse_ino_t ino;

  struct stat i_st;

  bool is_regular() const;
  bool is_directory() const;
  bool is_symlink() const;

  long int krefs = 0;

 protected:
  FileSystem *fs_;
};

class RegInode : public Inode {
 public:
  RegInode(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid,
      blksize_t blksize, mode_t mode, FileSystem *fs) :
    Inode(ino, time, uid, gid, blksize, mode, fs)
  {
    i_st.st_nlink = 1;
    i_st.st_mode = S_IFREG | mode;
  }

  ~RegInode();

  std::map<off_t, Extent> extents_;
};

class DirInode : public Inode {
 public:
  typedef std::map<std::string, std::shared_ptr<Inode>> dir_t;

  DirInode(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid,
      blksize_t blksize, mode_t mode, FileSystem *fs) :
    Inode(ino, time, uid, gid, blksize, mode, fs)
  {
    i_st.st_nlink = 2;
    i_st.st_blocks = 1;
    i_st.st_mode = S_IFDIR | mode;
  }

  dir_t dentries;
};

class SymlinkInode : public Inode {
 public:
  SymlinkInode(fuse_ino_t ino, time_t time, uid_t uid, gid_t gid,
      blksize_t blksize, const std::string& link, FileSystem *fs) :
    Inode(ino, time, uid, gid, blksize, 0, fs)
  {
    i_st.st_mode = S_IFLNK;
    i_st.st_size = link.length();
    this->link = link;
  }

  std::string link;
};
