#include "filesystem.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>
#include <limits.h>
#include <unistd.h>
#include "inode.h"

struct FileHandle {
  std::shared_ptr<RegInode> in;
  int flags;

  FileHandle(std::shared_ptr<RegInode> in, int flags) :
    in(in), flags(flags)
  {}
};

FileSystem::FileSystem(size_t size,
    const std::shared_ptr<spdlog::logger>& log) :
  log_(log),
  next_ino_(FUSE_ROOT_ID)
{
  auto now = std::time(nullptr);

  auto root = std::make_shared<DirInode>(next_ino_++, now,
      getuid(), getgid(), 4096, 0755, this);

  add_inode(root);

  avail_bytes_ = size;

  memset(&stat, 0, sizeof(stat));
  stat.f_fsid = 983983;
  stat.f_namemax = PATH_MAX;
  stat.f_bsize = 4096;
  stat.f_frsize = 4096;
  stat.f_blocks = size / 4096;
  stat.f_files = 0;
  stat.f_bfree = stat.f_blocks;
  stat.f_bavail = stat.f_blocks;

  if (size < 1ULL<<20) {
    log_->info("creating {} byte file system", size);
  } else if (size < 1ULL<<30) {
    auto mbs = size / (1ULL<<20);
    log_->info("creating {} mb file system", mbs);
  } else if (size < 1ULL<<40) {
    auto gbs = size / (1ULL<<30);
    log_->info("creating {} gb file system", gbs);
  } else if (size < 1ULL<<50) {
    auto tbs = size / (1ULL<<40);
    log_->info("creating {} tb file system", tbs);
  } else {
    assert(0 == "oh yeh?");
  }

  if (!next_ino_.is_lock_free()) {
    log_->warn("inode number allocation may not be lock free");
  }
}

void FileSystem::add_inode(const std::shared_ptr<Inode>& inode)
{
  assert(inode->krefs == 0);
  inode->krefs++;
  auto res = inodes_.emplace(inode->ino, inode);
  assert(res.second); // check for duplicate ino
}

void FileSystem::get_inode(const std::shared_ptr<Inode>& inode)
{
  inode->krefs++;
  auto res = inodes_.emplace(inode->ino, inode);
  if (!res.second) {
    assert(inode->krefs > 0);
  } else {
    assert(inode->krefs == 0);
  }
}

void FileSystem::put_inode(fuse_ino_t ino, long int dec)
{
  auto it = inodes_.find(ino);
  assert(it != inodes_.end());
  assert(it->second->krefs > 0);
  it->second->krefs -= dec;
  assert(it->second->krefs >= 0);
  if (it->second->krefs == 0) {
    inodes_.erase(it);
  }
}

uint64_t FileSystem::nfiles() const
{
  uint64_t ret = 0;
  for (auto it = inodes_.begin(); it != inodes_.end(); it++)
    if (it->second->i_st.st_mode & S_IFREG)
      ret++;
  return ret;
}

void FileSystem::destroy()
{
  log_->info("shutting down file system");
  // note that according to the fuse documentation when the file system is
  // unmounted and shutdown all of the inode references implicitly drop to zero.
}

int FileSystem::create(fuse_ino_t parent_ino, const std::string& name, mode_t mode,
    int flags, struct stat *st, FileHandle **fhp, uid_t uid, gid_t gid)
{
  log_->debug("create parent {} name {} mode {} flags {} uid {} gid {}",
      parent_ino, name, mode, flags, uid, gid);

  if (name.length() > NAME_MAX) {
    log_->debug("create name {} too long", name);
    return -ENAMETOOLONG;
  }

  auto now = std::time(nullptr);

  auto in = std::make_shared<RegInode>(next_ino_++, now, uid, gid, 4096, S_IFREG | mode, this);
  auto fh = std::make_unique<FileHandle>(in, flags);

  std::lock_guard<std::mutex> l(mutex_);

  auto parent_in = dir_inode(parent_ino);
  DirInode::dir_t& children = parent_in->dentries;
  if (children.find(name) != children.end()) {
    log_->debug("create name {} already exists", name);
    return -EEXIST;
  }

  int ret = access(parent_in, W_OK, uid, gid);
  if (ret) {
    log_->debug("create name {} access denied ret {}", name, ret);
    return ret;
  }

  children[name] = in;
  add_inode(in);

  parent_in->i_st.st_ctime = now;
  parent_in->i_st.st_mtime = now;

  *st = in->i_st;
  *fhp = fh.release();

  log_->debug("created name {} with ino {}", name, in->ino);

  return 0;
}

int FileSystem::getattr(fuse_ino_t ino, struct stat *st, uid_t uid, gid_t gid)
{
  std::lock_guard<std::mutex> l(mutex_);

  auto in = inode(ino);

  *st = in->i_st;

  return 0;
}

int FileSystem::unlink(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid)
{
  std::lock_guard<std::mutex> l(mutex_);

  auto parent_in = dir_inode(parent_ino);
  DirInode::dir_t::const_iterator it = parent_in->dentries.find(name);
  if (it == parent_in->dentries.end())
    return -ENOENT;

  int ret = access(parent_in, W_OK, uid, gid);
  if (ret)
    return ret;

  auto in = it->second;

  // see unlink(2): EISDIR may be another case
  if (in->is_directory())
    return -EPERM;

  if (parent_in->i_st.st_mode & S_ISVTX) {
    if (uid && uid != in->i_st.st_uid && uid != parent_in->i_st.st_uid)
      return -EPERM;
  }

  auto now = std::time(nullptr);

  in->i_st.st_ctime = now;
  in->i_st.st_nlink--;

  parent_in->i_st.st_ctime = now;
  parent_in->i_st.st_mtime = now;
  parent_in->dentries.erase(it);

  return 0;
}

int FileSystem::lookup(fuse_ino_t parent_ino, const std::string& name, struct stat *st)
{
  std::lock_guard<std::mutex> l(mutex_);

  // FIXME: should this be -ENOTDIR or -ENOENT in some cases?
  auto parent_in = dir_inode(parent_ino);
  DirInode::dir_t::const_iterator it = parent_in->dentries.find(name);
  if (it == parent_in->dentries.end()) {
    log_->debug("lookup parent {} name {} not found", parent_ino, name);
    return -ENOENT;
  }

  auto in = it->second;

  // bump kernel inode cache reference count
  get_inode(in);

  *st = in->i_st;

  log_->debug("lookup parent {} name {} found {}", parent_ino, name, in->ino);

  return 0;
}

int FileSystem::open(fuse_ino_t ino, int flags, FileHandle **fhp, uid_t uid, gid_t gid)
{
  int mode = 0;
  if ((flags & O_ACCMODE) == O_RDONLY)
    mode = R_OK;
  else if ((flags & O_ACCMODE) == O_WRONLY)
    mode = W_OK;
  else if ((flags & O_ACCMODE) == O_RDWR)
    mode = R_OK | W_OK;

  if (!(mode & W_OK) && (flags & O_TRUNC)) {
    const int ret = -EACCES;
    log_->debug("open ino {} flags {} uid {} gid {} ret {}",
        ino, flags, uid, gid, ret);
    return ret;
  }

  std::lock_guard<std::mutex> l(mutex_);

  auto generic_in = inode(ino);
  auto in = std::dynamic_pointer_cast<RegInode>(generic_in);
  assert(in->is_regular());
  auto fh = std::make_unique<FileHandle>(in, flags);

  int ret = access(in, mode, uid, gid);
  if (ret) {
    log_->debug("open ino {} flags {} uid {} gid {} ret {}",
        ino, flags, uid, gid, ret);
    return ret;
  }

  if (flags & O_TRUNC) {
    ret = truncate(in, 0, uid, gid);
    if (ret) {
      log_->debug("open ino {} flags {} uid {} gid {} ret {}",
          ino, flags, uid, gid, ret);
      return ret;
    }
    auto now = std::time(nullptr);
    in->i_st.st_mtime = now;
    in->i_st.st_ctime = now;
  }

  *fhp = fh.release();

  log_->debug("open ino {} flags {} uid {} gid {} ret {}",
      ino, flags, uid, gid, 0);

  return 0;
}

void FileSystem::release(fuse_ino_t ino, FileHandle *fh)
{
  log_->debug("release ino {} fh {}", ino, (void*)fh);
  assert(fh);
  delete fh;
}

void FileSystem::forget(fuse_ino_t ino, long unsigned nlookup)
{
  log_->debug("forget ino {} nlookup {}", ino, nlookup);
  std::lock_guard<std::mutex> l(mutex_);
  put_inode(ino, nlookup);
}

ssize_t FileSystem::write_buf(FileHandle *fh, struct fuse_bufvec *bufv, off_t off)
{
  std::lock_guard<std::mutex> l(mutex_);

  auto gen_in = fh->in;
  assert(gen_in->is_regular());
  auto in = std::dynamic_pointer_cast<RegInode>(gen_in);

  size_t written = 0;

  for (size_t i = bufv->idx; i < bufv->count; i++) {
    struct fuse_buf *buf = bufv->buf + i;

    assert(!(buf->flags & FUSE_BUF_IS_FD));
    assert(!(buf->flags & FUSE_BUF_FD_RETRY));
    assert(!(buf->flags & FUSE_BUF_FD_SEEK));

    ssize_t ret;
    if (i == bufv->idx) {
      ret = write(in, off, buf->size - bufv->off, (char*)buf->mem + bufv->off);
      if (ret < 0)
        return ret;
      assert(buf->size > bufv->off);
      if (ret < (ssize_t)(buf->size - bufv->off))
        return written;
    } else {
      ret = write(in, off, buf->size, (char*)buf->mem);
      if (ret < 0)
        return ret;
      if (ret < (ssize_t)buf->size)
        return written;
    }
    off += ret;
    written += ret;
  }

  return written;
}

ssize_t FileSystem::read(FileHandle *fh, off_t offset,
    size_t size, char *buf)
{
  std::lock_guard<std::mutex> l(mutex_);

  std::shared_ptr<RegInode> in = std::dynamic_pointer_cast<RegInode>(fh->in);

  in->i_st.st_atime = std::time(nullptr);

  // reads that start past eof return nothing
  if (offset >= in->i_st.st_size || size == 0)
    return 0;

  // clip the read so that it doesn't pass eof
  size_t left;
  if ((off_t)(offset + size) > in->i_st.st_size)
    left = in->i_st.st_size - offset;
  else
    left = size;

  const size_t new_size = left;
  char *dst = buf;

  /*
   * find first segment that might intersect the read
   *
   * upper_bound(offset) will return a pointer to the first segment whose
   * offset is greater (>) than the target offset. Thus, the immediately
   * preceeding segment is the first that has an offset less than or equal to
   * (<=) the offset which is what we are interested in.
   *
   * 1) it == begin(): can't move backward
   * 2) it == end() / other: <= case described above
   */
  auto it = in->extents_.upper_bound(offset);
  if (it != in->extents_.begin()) { // not empty
    assert(!in->extents_.empty());
    --it;
  } else if (it == in->extents_.end()) { // empty
    assert(in->extents_.empty());
    memset(dst, 0, new_size);
    return new_size;
  }

  assert(it != in->extents_.end());
  off_t seg_offset = it->first;

  while (left) {
    // size of movement this round
    size_t done = 0;

    // read starts before the current segment. return zeros up until the
    // beginning of the segment or until we've completed the read.
    if (offset < seg_offset) {
      done = std::min(left, (size_t)(seg_offset - offset));
      memset(dst, 0, done);
    } else {
      const auto& extent = it->second;
      off_t seg_end_offset = seg_offset + extent.size;

      // fixme: there may be a case here where the end of file lands inside an
      // allocated extent, but logically it shoudl be returning zeros

      // read starts within the current segment. return valid data up until
      // the end of the segment or until we've completed the read.
      if (offset < seg_end_offset) {
        done = std::min(left, (size_t)(seg_end_offset - offset));

        size_t blkoff = offset - seg_offset;
        std::memcpy(dst, extent.buf.get() + blkoff, done);

      } else if (++it == in->extents_.end()) {
        seg_offset = offset + left;
        assert(offset < seg_offset);
        // assert that we'll be done
        continue;
      } else {
        seg_offset = it->first;
        continue;
      }
    }
    dst += done;
    offset += done;
    left -= done;
  }

  return new_size;
}

int FileSystem::mkdir(fuse_ino_t parent_ino, const std::string& name, mode_t mode,
    struct stat *st, uid_t uid, gid_t gid)
{
  if (name.length() > NAME_MAX) {
    const int ret = -ENAMETOOLONG;
    log_->debug("mkdir parent {} name {} mode {} uid {} gid {} ret {}",
        parent_ino, name, mode, uid, gid, ret);
    return ret;
  }

  auto now = std::time(nullptr);

  auto in = std::make_shared<DirInode>(next_ino_++, now, uid, gid, 4096, mode, this);

  std::lock_guard<std::mutex> l(mutex_);

  auto parent_in = dir_inode(parent_ino);
  DirInode::dir_t& children = parent_in->dentries;
  if (children.find(name) != children.end()) {
    const int ret = -EEXIST;
    log_->debug("mkdir parent {} name {} mode {} uid {} gid {} ret {}",
        parent_ino, name, mode, uid, gid, ret);
    return ret;
  }

  int ret = access(parent_in, W_OK, uid, gid);
  if (ret) {
    log_->debug("mkdir parent {} name {} mode {} uid {} gid {} ret {}",
        parent_ino, name, mode, uid, gid, ret);
    return ret;
  }

  children[name] = in;
  add_inode(in);

  parent_in->i_st.st_ctime = now;
  parent_in->i_st.st_mtime = now;
  parent_in->i_st.st_nlink++;

  *st = in->i_st;

  log_->debug("mkdir parent {} name {} mode {} uid {} gid {} ret {}",
      parent_ino, name, mode, uid, gid, 0);

  return 0;
}

int FileSystem::rmdir(fuse_ino_t parent_ino, const std::string& name,
    uid_t uid, gid_t gid)
{
  std::lock_guard<std::mutex> l(mutex_);

  auto parent_in = dir_inode(parent_ino);
  DirInode::dir_t& children = parent_in->dentries;
  DirInode::dir_t::const_iterator it = children.find(name);
  if (it == children.end()) {
    log_->debug("rmdir ENOENT parent {} name {} uid {} gid {}",
        parent_ino, name, uid, gid);
    return -ENOENT;
  }

  if (!it->second->is_directory()) {
    log_->debug("rmdir ENOTDIR parent {} name {} uid {} gid {}",
        parent_ino, name, uid, gid);
    return -ENOTDIR;
  }

  auto in = std::static_pointer_cast<DirInode>(it->second);

  if (in->dentries.size()) {
    log_->debug("rmdir ENOTEMPTY parent {} name {} uid {} gid {}",
        parent_ino, name, uid, gid);
    return -ENOTEMPTY;
  }

  if (parent_in->i_st.st_mode & S_ISVTX) {
    if (uid && uid != in->i_st.st_uid && uid != parent_in->i_st.st_uid) {
      log_->debug("rmdir EPERM parent {} name {} uid {} gid {}",
          parent_ino, name, uid, gid);
      return -EPERM;
    }
  }

  auto now = std::time(nullptr);

  parent_in->i_st.st_mtime = now;
  parent_in->i_st.st_ctime = now;
  parent_in->dentries.erase(it);
  parent_in->i_st.st_nlink--;

  log_->debug("rmdir parent {} name {} uid {} gid {}",
      parent_ino, name, uid, gid);

  return 0;
}

int FileSystem::rename(fuse_ino_t parent_ino, const std::string& name,
    fuse_ino_t newparent_ino, const std::string& newname,
    uid_t uid, gid_t gid)
{
  if (name.length() > NAME_MAX || newname.length() > NAME_MAX)
    return -ENAMETOOLONG;

  std::lock_guard<std::mutex> l(mutex_);

  // old
  auto parent_in = dir_inode(parent_ino);
  DirInode::dir_t& parent_children = parent_in->dentries;
  DirInode::dir_t::const_iterator old_it = parent_children.find(name);
  if (old_it == parent_children.end())
    return -ENOENT;

  auto old_in = old_it->second;
  assert(old_in);

  // new
  auto newparent_in = dir_inode(newparent_ino);
  DirInode::dir_t& newparent_children = newparent_in->dentries;
  DirInode::dir_t::const_iterator new_it = newparent_children.find(newname);

  std::shared_ptr<Inode> new_in = NULL;
  if (new_it != newparent_children.end()) {
    new_in = new_it->second;
    assert(new_in);
  }

  /*
   * EACCES write permission is denied for the directory containing oldpath or
   * newpath,
   *
   * (TODO) or search permission is denied for one of the directories in the
   * path prefix  of  old‐ path or newpath,
   *
   * or oldpath is a directory and does not allow write permission (needed
   * to update the ..  entry).  (See also path_resolution(7).) TODO: this is
   * implemented but what is the affect on ".." update?
   */
  int ret = access(parent_in, W_OK, uid, gid);
  if (ret)
    return ret;

  ret = access(newparent_in, W_OK, uid, gid);
  if (ret)
    return ret;

  if (old_in->i_st.st_mode & S_IFDIR) {
    ret = access(old_in, W_OK, uid, gid);
    if (ret)
      return ret;
  }

  /*
   * EPERM or EACCES The  directory  containing  oldpath  has the sticky bit
   * (S_ISVTX) set and the process's effective user ID is neither the user ID
   * of the file to be deleted nor that of the directory containing it, and
   * the process is not privileged (Linux: does not have the CAP_FOWNER
   * capability);
   *
   * or newpath is an existing file and the directory containing it has the
   * sticky bit set and the process's effective user ID is neither the user
   * ID of the  file to  be  replaced  nor that of the directory containing
   * it, and the process is not privileged (Linux: does not have the
   * CAP_FOWNER capability);
   *
   * or the filesystem containing pathname does not support renaming of the
   * type requested.
   */
  if (parent_in->i_st.st_mode & S_ISVTX) {
    if (uid && uid != old_in->i_st.st_uid && uid != parent_in->i_st.st_uid)
      return -EPERM;
  }

  if (new_in &&
      newparent_in->i_st.st_mode & S_ISVTX &&
      uid && uid != new_in->i_st.st_uid &&
      uid != newparent_in->i_st.st_uid) {
    return -EPERM;
  }


  if (new_in) {
    if (old_in->i_st.st_mode & S_IFDIR) {
      if (new_in->i_st.st_mode & S_IFDIR) {
        DirInode::dir_t& new_children =
          std::static_pointer_cast<DirInode>(new_in)->dentries;
        if (new_children.size())
          return -ENOTEMPTY;
      } else
        return -ENOTDIR;
    } else {
      if (new_in->i_st.st_mode & S_IFDIR)
        return -EISDIR;
    }

    newparent_children.erase(new_it);
  }

  old_in->i_st.st_ctime = std::time(nullptr);

  newparent_children[newname] = old_it->second;
  parent_children.erase(old_it);

  return 0;
}

int FileSystem::setattr(fuse_ino_t ino, FileHandle *fh, struct stat *attr,
    int to_set, uid_t uid, gid_t gid)
{
  std::lock_guard<std::mutex> l(mutex_);
  mode_t clear_mode = 0;

  auto in = inode(ino);

  auto now = std::time(nullptr);

  if (to_set & FUSE_SET_ATTR_MODE) {
    if (uid && in->i_st.st_uid != uid)
      return -EPERM;

    if (uid && in->i_st.st_gid != gid)
      clear_mode |= S_ISGID;

    in->i_st.st_mode = attr->st_mode;
  }

  if (to_set & (FUSE_SET_ATTR_UID | FUSE_SET_ATTR_GID)) {
    /*
     * Only  a  privileged  process  (Linux: one with the CAP_CHOWN capability)
     * may change the owner of a file.  The owner of a file may change the
     * group of the file to any group of which that owner is a member.  A
     * privileged process (Linux: with CAP_CHOWN) may change the group
     * arbitrarily.
     *
     * TODO: group membership for owner is not enforced.
     */
    if (uid && (to_set & FUSE_SET_ATTR_UID) &&
        (in->i_st.st_uid != attr->st_uid))
      return -EPERM;

    if (uid && (to_set & FUSE_SET_ATTR_GID) &&
        (uid != in->i_st.st_uid))
      return -EPERM;

    if (to_set & FUSE_SET_ATTR_UID)
      in->i_st.st_uid = attr->st_uid;

    if (to_set & FUSE_SET_ATTR_GID)
      in->i_st.st_gid = attr->st_gid;
  }

  if (to_set & (FUSE_SET_ATTR_MTIME | FUSE_SET_ATTR_ATIME)) {
    if (uid && in->i_st.st_uid != uid)
      return -EPERM;

#ifdef FUSE_SET_ATTR_MTIME_NOW
    if (to_set & FUSE_SET_ATTR_MTIME_NOW)
      in->i_st.st_mtime = std::time(nullptr);
    else
#endif
    if (to_set & FUSE_SET_ATTR_MTIME)
      in->i_st.st_mtime = attr->st_mtime;

#ifdef FUSE_SET_ATTR_ATIME_NOW
    if (to_set & FUSE_SET_ATTR_ATIME_NOW)
      in->i_st.st_atime = std::time(nullptr);
    else
#endif
    if (to_set & FUSE_SET_ATTR_ATIME)
      in->i_st.st_atime = attr->st_atime;
  }

#ifdef FUSE_SET_ATTR_CTIME
  if (to_set & FUSE_SET_ATTR_CTIME) {
    if (uid && in->i_st.st_uid != uid)
      return -EPERM;
    in->i_st.st_ctime = attr->st_ctime;
  }
#endif

  if (to_set & FUSE_SET_ATTR_SIZE) {

    if (uid) {   // not root
      if (!fh) { // not open file descriptor
        int ret = access(in, W_OK, uid, gid);
        if (ret)
          return ret;
      } else if (((fh->flags & O_ACCMODE) != O_WRONLY) &&
                 ((fh->flags & O_ACCMODE) != O_RDWR)) {
        return -EACCES;
      }
    }

    // impose maximum size of 2TB
    if (attr->st_size > 2199023255552)
      return -EFBIG;

    assert(in->is_regular());
    auto reg_in = std::dynamic_pointer_cast<RegInode>(in);
    int ret = truncate(reg_in, attr->st_size, uid, gid);
    if (ret < 0)
      return ret;

    in->i_st.st_mtime = now;
  }

  in->i_st.st_ctime = now;

  if (to_set & FUSE_SET_ATTR_MODE)
    in->i_st.st_mode &= ~clear_mode;

  *attr = in->i_st;

  return 0;
}

int FileSystem::symlink(const std::string& link, fuse_ino_t parent_ino,
    const std::string& name, struct stat *st, uid_t uid, gid_t gid)
{
  if (name.length() > NAME_MAX)
    return -ENAMETOOLONG;

  auto now = std::time(nullptr);

  auto in = std::make_shared<SymlinkInode>(next_ino_++, now, uid, gid, 4096, link, this);

  std::lock_guard<std::mutex> l(mutex_);

  auto parent_in = dir_inode(parent_ino);
  DirInode::dir_t& children = parent_in->dentries;
  if (children.find(name) != children.end())
    return -EEXIST;

  int ret = access(parent_in, W_OK, uid, gid);
  if (ret)
    return ret;

  children[name] = in;
  add_inode(in);

  parent_in->i_st.st_ctime = now;
  parent_in->i_st.st_mtime = now;

  *st = in->i_st;

  return 0;
}

ssize_t FileSystem::readlink(fuse_ino_t ino, char *path, size_t maxlen, uid_t uid, gid_t gid)
{
  std::lock_guard<std::mutex> l(mutex_);

  auto in = symlink_inode(ino);
  size_t link_len = in->link.size();
  ssize_t ret;

  if (link_len > maxlen) {
    ret = -ENAMETOOLONG;
  } else {
    in->link.copy(path, link_len, 0);
    ret = link_len;
  }

  log_->debug("readlink ino {} path {} maxlen {} uid {} gid {} ret {}",
      ino, path, maxlen, uid, gid, ret);

  return ret;
}

int FileSystem::statfs(fuse_ino_t ino, struct statvfs *stbuf)
{
  std::lock_guard<std::mutex> l(mutex_);

  // assert we are in this file system
  auto in = inode(ino);
  (void)in;

  stat.f_files = nfiles();
  stat.f_bfree = avail_bytes_ / 4096;
  stat.f_bavail = avail_bytes_ / 4096;

  *stbuf = stat;

  return 0;
}

int FileSystem::link(fuse_ino_t ino, fuse_ino_t newparent_ino, const std::string& newname,
    struct stat *st, uid_t uid, gid_t gid)
{
  if (newname.length() > NAME_MAX)
    return -ENAMETOOLONG;

  std::lock_guard<std::mutex> l(mutex_);

  auto newparent_in = dir_inode(newparent_ino);
  if (newparent_in->dentries.find(newname) != newparent_in->dentries.end())
    return -EEXIST;

  auto in = inode(ino);

  if (in->i_st.st_mode & S_IFDIR)
    return -EPERM;

  int ret = access(newparent_in, W_OK, uid, gid);
  if (ret)
    return ret;

  auto now = std::time(nullptr);

  // bump in kernel inode cache reference count
  get_inode(in);

  in->i_st.st_ctime = now;
  in->i_st.st_nlink++;

  newparent_in->i_st.st_ctime = now;
  newparent_in->i_st.st_mtime = now;
  newparent_in->dentries[newname] = in;

  *st = in->i_st;

  return 0;
}

int FileSystem::access(const std::shared_ptr<Inode>& in, int mask, uid_t uid, gid_t gid)
{
  if (mask == F_OK)
    return 0;

  assert(mask & (R_OK | W_OK | X_OK));

  if (in->i_st.st_uid == uid) {
    if (mask & R_OK) {
      if (!(in->i_st.st_mode & S_IRUSR))
        return -EACCES;
    }
    if (mask & W_OK) {
      if (!(in->i_st.st_mode & S_IWUSR))
        return -EACCES;
    }
    if (mask & X_OK) {
      if (!(in->i_st.st_mode & S_IXUSR))
        return -EACCES;
    }
    return 0;
  } else if (in->i_st.st_gid == gid) {
    if (mask & R_OK) {
      if (!(in->i_st.st_mode & S_IRGRP))
        return -EACCES;
    }
    if (mask & W_OK) {
      if (!(in->i_st.st_mode & S_IWGRP))
        return -EACCES;
    }
    if (mask & X_OK) {
      if (!(in->i_st.st_mode & S_IXGRP))
        return -EACCES;
    }
    return 0;
  } else if (uid == 0) {
    if (mask & X_OK) {
      if (!(in->i_st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
        return -EACCES;
    }
    return 0;
  } else {
    if (mask & R_OK) {
      if (!(in->i_st.st_mode & S_IROTH))
        return -EACCES;
    }
    if (mask & W_OK) {
      if (!(in->i_st.st_mode & S_IWOTH))
        return -EACCES;
    }
    if (mask & X_OK) {
      if (!(in->i_st.st_mode & S_IXOTH))
        return -EACCES;
    }
    return 0;
  }

  assert(0);
}


int FileSystem::access(fuse_ino_t ino, int mask, uid_t uid, gid_t gid)
{
  std::lock_guard<std::mutex> l(mutex_);

  auto in = inode(ino);

  return access(in, mask, uid, gid);
}

/*
 * Allow mknod to create special files, but enforce that these files are
 * never used in anything other than metadata operations.
 *
 * TODO: add checks that enforce non-use of special files. Note that this
 * routine can also create regular files.
 */
int FileSystem::mknod(fuse_ino_t parent_ino, const std::string& name, mode_t mode,
    dev_t rdev, struct stat *st, uid_t uid, gid_t gid)
{
  if (name.length() > NAME_MAX)
    return -ENAMETOOLONG;

  auto now = std::time(nullptr);

  // TODO: may not be Regular Inode?
  auto in = std::make_shared<RegInode>(next_ino_++, now, uid, gid, 4096, mode, this);

  // directories start with nlink = 2, but according to mknod(2), "Under
  // Linux, mknod() cannot be used to create directories.  One should make
  // directories with mkdir(2).".
  assert(!in->is_directory());

  std::lock_guard<std::mutex> l(mutex_);

  auto parent_in = dir_inode(parent_ino);
  DirInode::dir_t& children = parent_in->dentries;
  if (children.find(name) != children.end())
    return -EEXIST;

  int ret = access(parent_in, W_OK, uid, gid);
  if (ret)
    return ret;

  children[name] = in;
  add_inode(in);

  parent_in->i_st.st_ctime = now;
  parent_in->i_st.st_mtime = now;

  *st = in->i_st;

  return 0;
}

int FileSystem::opendir(fuse_ino_t ino, int flags, uid_t uid, gid_t gid)
{
  std::lock_guard<std::mutex> l(mutex_);

  auto in = inode(ino);

  if ((flags & O_ACCMODE) == O_RDONLY) {
    int ret = access(in, R_OK, uid, gid);
    if (ret)
      return ret;
  }

  return 0;
}

/*
 * This is a work-in-progress. It currently is functioning, but I think that
 * the it is not robust against concurrent modifications. The common
 * approach it seems is to encode a cookie in the offset parameter. Current
 * we just do an in-order traversal of the directory and return the Nth
 * item.
 */
ssize_t FileSystem::readdir(fuse_req_t req, fuse_ino_t ino, char *buf,
    size_t bufsize, off_t off)
{
  std::lock_guard<std::mutex> l(mutex_);

  struct stat st;
  memset(&st, 0, sizeof(st));

  size_t pos = 0;

  /*
   * FIXME: the ".." directory correctly shows up at the parent directory
   * inode, but "." shows a inode number as "?" with ls -lia.
   */
  if (off == 0) {
    size_t remaining = bufsize - pos;
    memset(&st, 0, sizeof(st));
    st.st_ino = 1;
    size_t used = fuse_add_direntry(req, buf + pos, remaining, ".", &st, 1);
    if (used > remaining)
      return pos;
    pos += used;
    off = 1;
  }

  if (off == 1) {
    size_t remaining = bufsize - pos;
    memset(&st, 0, sizeof(st));
    st.st_ino = 1;
    size_t used = fuse_add_direntry(req, buf + pos, remaining, "..", &st, 2);
    if (used > remaining)
      return pos;
    pos += used;
    off = 2;
  }

  assert(off >= 2);

  auto dir_in = dir_inode(ino);
  const DirInode::dir_t& children = dir_in->dentries;

  size_t count = 0;
  size_t target = off - 2;

  for (DirInode::dir_t::const_iterator it = children.begin();
      it != children.end(); it++) {
    if (count >= target) {
      auto in = it->second;
      assert(in);
      memset(&st, 0, sizeof(st));
      st.st_ino = in->i_st.st_ino;
      size_t remaining = bufsize - pos;
      size_t used = fuse_add_direntry(req, buf + pos, remaining, it->first.c_str(), &st, off + 1);
      if (used > remaining)
        return pos;
      pos += used;
      off++;
    }
    count++;
  }

  return pos;
}

void FileSystem::releasedir(fuse_ino_t ino) {}

void FileSystem::free_space(Extent *extent)
{
  extent->buf.release();
  avail_bytes_ += extent->size;
}

int FileSystem::truncate(const std::shared_ptr<RegInode>& in, off_t newsize, uid_t uid, gid_t gid)
{
  // easy: nothing to do
  if (in->i_st.st_size == newsize) {
    return 0;

  // easy: free all extents
  } else if (newsize == 0) {
    for (auto& it : in->extents_) {
      free_space(&it.second);
    }
    in->extents_.clear();
    in->i_st.st_size = 0;

  // shrink file. the basic strategy is to free all extents past newsize
  // offset. we have to be careful if newsize falls into an extent and not
  // free that extent.
  } else if (newsize < in->i_st.st_size) {
    // find extent that could intersect newsize
    auto it = in->extents_.upper_bound(newsize);
    if (it != in->extents_.begin()) {
      assert(!in->extents_.empty());
      --it;
    } else if (it == in->extents_.end()) {
      // empty: this case could happen if a file was truncated to be large,
      // then truncated to be small without the file having any space
      // allocated to it (i.e. no writes were performed). in this case we can
      // just set the file size (zero fill happens during read).
      in->i_st.st_size = newsize;
      return 0;
    }

    assert(it != in->extents_.end());
    off_t extent_offset = it->first;

    // newsize lands before the extent, so this extent and all extents after
    // it can be freed. if newsize falls within a hole, then zero fill will be
    // handled correctly during read. if newsize == extent_offset then the
    // actual last byte falls before the extent and we still remove it.
    if (newsize <= extent_offset) {
      for (auto it2 = it; it2 != in->extents_.end(); it2++) {
        free_space(&it2->second);
      }
      in->extents_.erase(it, in->extents_.end());
      in->i_st.st_size = newsize;
      return 0;
    }

    const auto& extent = it->second;
    off_t extent_end = extent_offset + extent.size;

    if (newsize <= extent_end)
      it++;

    for (auto it2 = it; it2 != in->extents_.end(); it2++) {
      free_space(&it2->second);
    }

    in->extents_.erase(it, in->extents_.end());
    in->i_st.st_size = newsize;

    return 0;

  // expand file with zeros. the basic strategy here is create a big hole
  // after the end of the file which will be zero filled during read. if the
  // current end of file lands in an extent, then zero fill that because read
  // can't tell if allocated space is part of that "hole".
  } else {
    assert(in->i_st.st_size < newsize);

    // find extent that could intersect newsize
    auto it = in->extents_.upper_bound(in->i_st.st_size);
    if (it != in->extents_.begin()) {
      assert(!in->extents_.empty());
      assert(it == in->extents_.end());
      --it;
    } else if (it == in->extents_.end()) {
      // empty: could happen with small truncate then large truncate having
      // not yet allocated any space (i.e. no writes).
      in->i_st.st_size = newsize;
      return 0;
    }

    assert(it != in->extents_.end());
    off_t extent_offset = it->first;

    assert(in->i_st.st_size >= extent_offset);

    const auto& extent = it->second;
    off_t extent_end = extent_offset + extent.size;

    if (extent_end < in->i_st.st_size) {
      in->i_st.st_size = newsize;
      return 0;
    }

    size_t left = std::min(extent_end - in->i_st.st_size,
        newsize - in->i_st.st_size);
    assert(left);

    char zeros[4096];
    memset(zeros, 0, sizeof(zeros));

    while (left) {
      size_t done = std::min(left, sizeof(zeros));
      ssize_t ret = write(in, in->i_st.st_size, done, zeros);
      assert(ret > 0);
      left -= ret;
    }

    in->i_st.st_size = newsize;
  }

  return 0;
}

/*
 * Allocate storage space for a file. The space should be available at file
 * offset @offset, and be no larger than @size bytes.
 */
int FileSystem::allocate_space(RegInode *in,
    std::map<off_t, Extent>::iterator *it,
    off_t offset, size_t size, bool upper_bound)
{
  // cap allocation size at 1mb, and if it isn't just filling a hole, then
  // make sure there is a lower bound on allocation size.
  size = std::min(size, (size_t)(1ULL<<20));
  if (!upper_bound)
    size = std::max(size, (size_t)8192);

  // allocate some space
  if (avail_bytes_ < size)
    return -ENOSPC;

  avail_bytes_ -= size;

  auto ret = in->extents_.emplace(offset, Extent(size));
  assert(ret.second);
  *it = ret.first;

  return 0;
}

ssize_t FileSystem::write(const std::shared_ptr<RegInode>& in, off_t offset, size_t size, const char *buf)
{
  auto now = std::time(nullptr);
  in->i_st.st_ctime = now;
  in->i_st.st_mtime = now;

  // find the first extent that could intersect the write
  auto it = in->extents_.upper_bound(offset);
  if (it != in->extents_.begin()) {
    assert(!in->extents_.empty());
    --it;
  } else if (it == in->extents_.end()) {
    assert(in->extents_.empty());
    int ret = allocate_space(in.get(), &it, offset, size, false);
    if (ret)
      return ret;
  }

  assert(it != in->extents_.end());
  off_t seg_offset = it->first;

  size_t left = size;

  while (left) {
    // case 1. the offset is contained in a non-allocated region before the
    // extent. allocate some space starting at the target offset that doesn't
    // extend past the beginning of the extent.
    if (offset < seg_offset) {
#if 0
      std::cout << "write:case1: offset=" << offset <<
        " size=" << (seg_offset - offset) <<
        " upper_bound=true"
        << std::endl;
#endif
      int ret = allocate_space(in.get(), &it, offset, seg_offset - offset, true);
      if (ret)
        return ret;

      seg_offset = it->first;

      continue;
    }

    const auto& extent = it->second;
    off_t seg_end_offset = seg_offset + extent.size;

    // case 2. the offset falls within the current extent: write data
    if (offset < seg_end_offset) {
      size_t done = std::min(left, (size_t)(seg_end_offset - offset));
      size_t blkoff = offset - seg_offset;

      std::memcpy(extent.buf.get() + blkoff, buf, done);

      buf += done;
      offset += done;
      left -= done;

      in->i_st.st_size = std::max(in->i_st.st_size, offset);

      continue;
    }

    // case 3. the offset falls past the extent, and there are no more
    // extents. in this case we extend the file allocation.
    if (++it == in->extents_.end()) {
      int ret = allocate_space(in.get(), &it, offset, left, false);
      if (ret)
        return ret;

      seg_offset = it->first;

      continue;
    }

    // case 4. try the next extent
    seg_offset = it->first;
  }

  return size;
}
