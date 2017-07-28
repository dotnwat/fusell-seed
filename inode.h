#ifndef GASSYFS_INODE_H_
#define GASSYFS_INODE_H_
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

  Inode(time_t time, uid_t uid, gid_t gid, blksize_t blksize,
      mode_t mode, FileSystem *fs);
  virtual ~Inode();

  void set_ino(fuse_ino_t ino);
  fuse_ino_t ino() const;

  struct stat i_st;

  bool is_directory() const;
  bool is_symlink() const;

  std::map<off_t, Extent> extents_;

 private:
  bool ino_set_;
  fuse_ino_t ino_;
  FileSystem *fs_;
};

// TODO: specialize for regular file

class DirInode : public Inode {
 public:
  typedef std::shared_ptr<DirInode> Ptr;
  typedef std::map<std::string, Inode::Ptr> dir_t;
  DirInode(time_t time, uid_t uid, gid_t gid, blksize_t blksize,
      mode_t mode, FileSystem *fs) :
    Inode(time, uid, gid, blksize, mode, fs) {
      i_st.st_nlink = 2;
      i_st.st_mode = S_IFDIR | mode;
      i_st.st_blocks = 1;
    }
  dir_t dentries;
};

class SymlinkInode : public Inode {
 public:
  typedef std::shared_ptr<SymlinkInode> Ptr;
  SymlinkInode(time_t time, uid_t uid, gid_t gid, blksize_t blksize,
      const std::string& link, FileSystem *fs) :
    Inode(time, uid, gid, blksize, 0, fs) {
      i_st.st_mode = S_IFLNK;
      this->link = link;
      i_st.st_size = link.length();
    }
  std::string link;
};

#endif
