#pragma once

#include <atomic>
#include <cassert>
#include <fuse.h>
#include <fuse_lowlevel.h>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>

#include "inode.h"
#include "spdlog/spdlog.h"

struct FileHandle;

class FileSystem {
public:
    FileSystem(size_t size, const std::shared_ptr<spdlog::logger>& log);

    FileSystem(const FileSystem& other) = delete;
    FileSystem(FileSystem&& other) = delete;
    ~FileSystem() = default;
    FileSystem& operator=(const FileSystem& other) = delete;
    FileSystem& operator=(const FileSystem&& other) = delete;

public:
    void destroy();
    int lookup(fuse_ino_t parent_ino, const std::string& name, struct stat* st);
    void forget(fuse_ino_t ino, long unsigned nlookup);
    int statfs(fuse_ino_t ino, struct statvfs* stbuf);

    // TODO: get rid of this method
    void free_space(Extent* extent);

    // inode operations
public:
    int mknod(
      fuse_ino_t parent_ino,
      const std::string& name,
      mode_t mode,
      dev_t rdev,
      struct stat* st,
      uid_t uid,
      gid_t gid);

    int symlink(
      const std::string& link,
      fuse_ino_t parent_ino,
      const std::string& name,
      struct stat* st,
      uid_t uid,
      gid_t gid);

    int link(
      fuse_ino_t ino,
      fuse_ino_t newparent_ino,
      const std::string& newname,
      struct stat* st,
      uid_t uid,
      gid_t gid);

    int rename(
      fuse_ino_t parent_ino,
      const std::string& name,
      fuse_ino_t newparent_ino,
      const std::string& newname,
      uid_t uid,
      gid_t gid);

    int unlink(
      fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid);

    int access(fuse_ino_t ino, int mask, uid_t uid, gid_t gid);

    int getattr(fuse_ino_t ino, struct stat* st, uid_t uid, gid_t gid);

    int setattr(
      fuse_ino_t ino,
      FileHandle* fh,
      struct stat* attr,
      int to_set,
      uid_t uid,
      gid_t gid);

    ssize_t
    readlink(fuse_ino_t ino, char* path, size_t maxlen, uid_t uid, gid_t gid);

    // directory operations
public:
    int mkdir(
      fuse_ino_t parent_ino,
      const std::string& name,
      mode_t mode,
      struct stat* st,
      uid_t uid,
      gid_t gid);

    int opendir(fuse_ino_t ino, int flags, uid_t uid, gid_t gid);

    ssize_t readdir(
      fuse_req_t req, fuse_ino_t ino, char* buf, size_t bufsize, off_t off);

    int
    rmdir(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid);

    void releasedir(fuse_ino_t ino);

    // file handle operation
public:
    int create(
      fuse_ino_t parent_ino,
      const std::string& name,
      mode_t mode,
      int flags,
      struct stat* st,
      FileHandle** fhp,
      uid_t uid,
      gid_t gid);

    int open(fuse_ino_t ino, int flags, FileHandle** fhp, uid_t uid, gid_t gid);
    ssize_t write_buf(FileHandle* fh, struct fuse_bufvec* bufv, off_t off);
    ssize_t read(FileHandle* fh, off_t offset, size_t size, char* buf);
    void release(fuse_ino_t ino, FileHandle* fh);

private:
    std::mutex mutex_;
    std::shared_ptr<spdlog::logger> log_;

    // inode management
private:
    void add_inode(const std::shared_ptr<Inode>& inode);
    void get_inode(const std::shared_ptr<Inode>& inode);
    void put_inode(fuse_ino_t ino, long int dec);

    std::shared_ptr<Inode> inode(fuse_ino_t ino) {
        return inodes_.at(ino);
        ;
    }

    std::shared_ptr<DirInode> dir_inode(fuse_ino_t ino) {
        auto in = inode(ino);
        assert(in->is_directory());
        return std::static_pointer_cast<DirInode>(in);
    }

    std::shared_ptr<SymlinkInode> symlink_inode(fuse_ino_t ino) {
        auto in = inode(ino);
        assert(in->is_symlink());
        return std::static_pointer_cast<SymlinkInode>(in);
    }

    std::atomic<fuse_ino_t> next_ino_;
    std::unordered_map<fuse_ino_t, std::shared_ptr<Inode>> inodes_;

    // helpers
private:
    // TODO: probably do not need to pass shared ptr here
    ssize_t write(
      const std::shared_ptr<RegInode>& in,
      off_t offset,
      size_t size,
      const char* buf);

    int
    access(const std::shared_ptr<Inode>& in, int mask, uid_t uid, gid_t gid);

    int truncate(
      const std::shared_ptr<RegInode>& in, off_t newsize, uid_t uid, gid_t gid);

    int allocate_space(
      RegInode* in,
      std::map<off_t, Extent>::iterator* it,
      off_t offset,
      size_t size,
      bool upper_bound);

    uint64_t nfiles() const;

    struct statvfs stat;
    size_t avail_bytes_;
};
