#pragma once
#include <cassert>
#include <cstring>
#include <fuse_lowlevel.h>
#include <memory>

#include "filesystem.h"

namespace foo {

template<typename T>
class default_lowlevel_ops {
public:
    static T* get(void* userdata) { return reinterpret_cast<T*>(userdata); }
    static T* get(fuse_req_t req) { return get(fuse_req_userdata(req)); }

    static void destroy(void* userdata)
        requires destroy<T>;

    static void create(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      mode_t mode,
      struct fuse_file_info* fi)
        requires create<T>;

    static void
    release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires release<T>;

    static void unlink(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires unlink<T>;

    static void forget(fuse_req_t req, fuse_ino_t ino, long unsigned nlookup)
        requires forget<T>;

    // TODO
    static void forget_multi(
      fuse_req_t req, size_t count, struct fuse_forget_data* forgets);

    static void
    getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires get_attribute<T>;

    static void lookup(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires lookup<T>;

    static void
    opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires open_directory<T>;

    static void readdir(
      fuse_req_t req,
      fuse_ino_t ino,
      size_t size,
      off_t off,
      struct fuse_file_info* fi)
        requires read_directory<T>;

    static void
    releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires release_directory<T>;

    static void open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires open<T>;

    static void write_buf(
      fuse_req_t req,
      fuse_ino_t ino,
      struct fuse_bufvec* bufv,
      off_t off,
      struct fuse_file_info* fi)
        requires write_buffer<T>;

    static void read(
      fuse_req_t req,
      fuse_ino_t ino,
      size_t size,
      off_t off,
      struct fuse_file_info* fi)
        requires read<T>;

    static void
    mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode)
        requires make_directory<T>;

    static void rmdir(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires remove_directory<T>;

    static void rename(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      fuse_ino_t newparent,
      const char* newname)
        requires rename<T>;

    static void setattr(
      fuse_req_t req,
      fuse_ino_t ino,
      struct stat* attr,
      int to_set,
      struct fuse_file_info* fi)
        requires set_attribute<T>;

    static void readlink(fuse_req_t req, fuse_ino_t ino)
        requires read_symlink<T>;

    static void symlink(
      fuse_req_t req, const char* link, fuse_ino_t parent, const char* name)
        requires make_symlink<T>;

    // TODO
    static void fsync(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi);

    // TODO
    static void fsyncdir(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi);

    static void statfs(fuse_req_t req, fuse_ino_t ino)
        requires stat_filesystem<T>;

    static void link(
      fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char* newname)
        requires make_hard_link<T>;

    static void access(fuse_req_t req, fuse_ino_t ino, int mask)
        requires access<T>;

    static void mknod(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      mode_t mode,
      dev_t rdev)
        requires make_node<T>;

    // TODO
    static void fallocate(
      fuse_req_t req,
      fuse_ino_t ino,
      int mode,
      off_t offset,
      off_t length,
      struct fuse_file_info* fi);
};

template<typename T>
void default_lowlevel_ops<T>::destroy(void* userdata)
    requires ::foo::destroy<T>
{
    auto fs = get(userdata);
    fs->destroy();
}

template<typename T>
void default_lowlevel_ops<T>::create(
  fuse_req_t req,
  fuse_ino_t parent,
  const char* name,
  mode_t mode,
  struct fuse_file_info* fi)
    requires ::foo::create<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    struct fuse_entry_param fe;
    std::memset(&fe, 0, sizeof(fe));

    FileHandle* fh;
    int ret = fs->create(
      parent, name, mode, fi->flags, &fe.attr, &fh, ctx->uid, ctx->gid);
    if (ret == 0) {
        fi->fh = reinterpret_cast<uint64_t>(fh);
        fe.ino = fe.attr.st_ino;
        fe.generation = 0;
        fe.entry_timeout = 1.0;
        fuse_reply_create(req, &fe, fi);
    } else {
        fuse_reply_err(req, -ret);
    }
}

template<typename T>
void default_lowlevel_ops<T>::release(
  fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
    requires ::foo::release<T>
{
    auto fs = get(req);
    auto fh = reinterpret_cast<FileHandle*>(fi->fh);

    fs->release(ino, fh);
    fuse_reply_err(req, 0);
}

template<typename T>
void default_lowlevel_ops<T>::unlink(
  fuse_req_t req, fuse_ino_t parent, const char* name)
    requires ::foo::unlink<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    int ret = fs->unlink(parent, name, ctx->uid, ctx->gid);
    fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::forget(
  fuse_req_t req, fuse_ino_t ino, long unsigned nlookup)
    requires ::foo::forget<T>
{
    auto fs = get(req);

    fs->forget(ino, nlookup);
    fuse_reply_none(req);
}

// TODO
template<typename T>
void default_lowlevel_ops<T>::forget_multi(
  fuse_req_t req, size_t count, struct fuse_forget_data* forgets) {
    auto fs = get(req);

    for (size_t i = 0; i < count; i++) {
        const struct fuse_forget_data* f = forgets + i;
        fs->forget(f->ino, f->nlookup);
    }

    fuse_reply_none(req);
}

template<typename T>
void default_lowlevel_ops<T>::getattr(
  fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
    requires get_attribute<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    struct stat st;
    int ret = fs->getattr(ino, &st, ctx->uid, ctx->gid);
    if (ret == 0)
        fuse_reply_attr(req, &st, ret);
    else
        fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::lookup(
  fuse_req_t req, fuse_ino_t parent, const char* name)
    requires ::foo::lookup<T>
{
    auto fs = get(req);

    struct fuse_entry_param fe;
    std::memset(&fe, 0, sizeof(fe));

    int ret = fs->lookup(parent, name, &fe.attr);
    if (ret == 0) {
        fe.ino = fe.attr.st_ino;
        fe.generation = 0;
        fuse_reply_entry(req, &fe);
    } else {
        fuse_reply_err(req, -ret);
    }
}

template<typename T>
void default_lowlevel_ops<T>::opendir(
  fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
    requires open_directory<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    int ret = fs->opendir(ino, fi->flags, ctx->uid, ctx->gid);
    if (ret == 0) {
        fuse_reply_open(req, fi);
    } else {
        fuse_reply_err(req, -ret);
    }
}

template<typename T>
void default_lowlevel_ops<T>::readdir(
  fuse_req_t req,
  fuse_ino_t ino,
  size_t size,
  off_t off,
  struct fuse_file_info* fi)
    requires read_directory<T>
{
    auto fs = get(req);

    auto buf = std::unique_ptr<char[]>(new char[size]);

    ssize_t ret = fs->readdir(req, ino, buf.get(), size, off);
    if (ret >= 0) {
        fuse_reply_buf(req, buf.get(), (size_t)ret);
    } else {
        int r = (int)ret;
        fuse_reply_err(req, -r);
    }
}

template<typename T>
void default_lowlevel_ops<T>::releasedir(
  fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
    requires release_directory<T>
{
    auto fs = get(req);

    fs->releasedir(ino);
    fuse_reply_err(req, 0);
}

template<typename T>
void default_lowlevel_ops<T>::open(
  fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
    requires ::foo::open<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    // new files are handled by create
    assert(!(fi->flags & O_CREAT));

    FileHandle* fh;
    int ret = fs->open(ino, fi->flags, &fh, ctx->uid, ctx->gid);
    if (ret == 0) {
        fi->fh = reinterpret_cast<uint64_t>(fh);
        fuse_reply_open(req, fi);
    } else {
        fuse_reply_err(req, -ret);
    }
}

template<typename T>
void default_lowlevel_ops<T>::write_buf(
  fuse_req_t req,
  fuse_ino_t ino,
  struct fuse_bufvec* bufv,
  off_t off,
  struct fuse_file_info* fi)
    requires write_buffer<T>
{
    auto fs = get(req);
    auto fh = reinterpret_cast<FileHandle*>(fi->fh);

    ssize_t ret = fs->write_buf(fh, bufv, off);
    if (ret >= 0)
        fuse_reply_write(req, ret);
    else
        fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::read(
  fuse_req_t req,
  fuse_ino_t ino,
  size_t size,
  off_t off,
  struct fuse_file_info* fi)
    requires ::foo::read<T>
{
    auto fs = get(req);
    auto fh = reinterpret_cast<FileHandle*>(fi->fh);

    auto buf = std::unique_ptr<char[]>(new char[size]);

    ssize_t ret = fs->read(fh, off, size, buf.get());
    if (ret >= 0)
        fuse_reply_buf(req, buf.get(), ret);
    else
        fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::mkdir(
  fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode)
    requires make_directory<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    struct fuse_entry_param fe;
    std::memset(&fe, 0, sizeof(fe));

    int ret = fs->mkdir(parent, name, mode, &fe.attr, ctx->uid, ctx->gid);
    if (ret == 0) {
        fe.ino = fe.attr.st_ino;
        fe.generation = 0;
        fe.entry_timeout = 1.0;
        fuse_reply_entry(req, &fe);
    } else {
        fuse_reply_err(req, -ret);
    }
}

template<typename T>
void default_lowlevel_ops<T>::rmdir(
  fuse_req_t req, fuse_ino_t parent, const char* name)
    requires remove_directory<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    int ret = fs->rmdir(parent, name, ctx->uid, ctx->gid);
    fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::rename(
  fuse_req_t req,
  fuse_ino_t parent,
  const char* name,
  fuse_ino_t newparent,
  const char* newname)
    requires ::foo::rename<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    int ret = fs->rename(parent, name, newparent, newname, ctx->uid, ctx->gid);
    fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::setattr(
  fuse_req_t req,
  fuse_ino_t ino,
  struct stat* attr,
  int to_set,
  struct fuse_file_info* fi)
    requires set_attribute<T>
{
    auto fs = get(req);
    auto fh = fi ? reinterpret_cast<FileHandle*>(fi->fh) : nullptr;
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    int ret = fs->setattr(ino, fh, attr, to_set, ctx->uid, ctx->gid);
    if (ret == 0)
        fuse_reply_attr(req, attr, 0);
    else
        fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::readlink(fuse_req_t req, fuse_ino_t ino)
    requires read_symlink<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);
    char path[PATH_MAX + 1];

    ssize_t ret = fs->readlink(ino, path, sizeof(path) - 1, ctx->uid, ctx->gid);
    if (ret >= 0) {
        path[ret] = '\0';
        fuse_reply_readlink(req, path);
    } else {
        int r = (int)ret;
        fuse_reply_err(req, -r);
    }
}

template<typename T>
void default_lowlevel_ops<T>::symlink(
  fuse_req_t req, const char* link, fuse_ino_t parent, const char* name)
    requires make_symlink<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    struct fuse_entry_param fe;
    std::memset(&fe, 0, sizeof(fe));

    int ret = fs->symlink(link, parent, name, &fe.attr, ctx->uid, ctx->gid);
    if (ret == 0) {
        fe.ino = fe.attr.st_ino;
        fuse_reply_entry(req, &fe);
    } else {
        fuse_reply_err(req, -ret);
    }
}

// TODO
template<typename T>
void default_lowlevel_ops<T>::fsync(
  fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi) {
    fuse_reply_err(req, 0);
}

// TODO
template<typename T>
void default_lowlevel_ops<T>::fsyncdir(
  fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi) {
    fuse_reply_err(req, 0);
}

template<typename T>
void default_lowlevel_ops<T>::statfs(fuse_req_t req, fuse_ino_t ino)
    requires stat_filesystem<T>
{
    auto fs = get(req);

    struct statvfs stbuf;
    std::memset(&stbuf, 0, sizeof(stbuf));

    int ret = fs->statfs(ino, &stbuf);
    if (ret == 0)
        fuse_reply_statfs(req, &stbuf);
    else
        fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::link(
  fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char* newname)
    requires make_hard_link<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    struct fuse_entry_param fe;
    std::memset(&fe, 0, sizeof(fe));

    int ret = fs->link(ino, newparent, newname, &fe.attr, ctx->uid, ctx->gid);
    if (ret == 0) {
        fe.ino = fe.attr.st_ino;
        fuse_reply_entry(req, &fe);
    } else {
        fuse_reply_err(req, -ret);
    }
}

template<typename T>
void default_lowlevel_ops<T>::access(fuse_req_t req, fuse_ino_t ino, int mask)
    requires ::foo::access<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    int ret = fs->access(ino, mask, ctx->uid, ctx->gid);
    fuse_reply_err(req, -ret);
}

template<typename T>
void default_lowlevel_ops<T>::mknod(
  fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode, dev_t rdev)
    requires make_node<T>
{
    auto fs = get(req);
    const struct fuse_ctx* ctx = fuse_req_ctx(req);

    struct fuse_entry_param fe;
    std::memset(&fe, 0, sizeof(fe));

    int ret = fs->mknod(parent, name, mode, rdev, &fe.attr, ctx->uid, ctx->gid);
    if (ret == 0) {
        fe.ino = fe.attr.st_ino;
        fuse_reply_entry(req, &fe);
    } else {
        fuse_reply_err(req, -ret);
    }
}

// TODO
template<typename T>
void default_lowlevel_ops<T>::fallocate(
  fuse_req_t req,
  fuse_ino_t ino,
  int mode,
  off_t offset,
  off_t length,
  struct fuse_file_info* fi) {
    fuse_reply_err(req, 0);
}

} // namespace foo
