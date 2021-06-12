#pragma once

#include <atomic>
#include <cassert>
#include <cstring>
#include <fuse.h>
#include <fuse_lowlevel.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <linux/limits.h>

struct FileHandle;

class filesystem_base {
public:
    filesystem_base() {
        std::memset(&ops_, 0, sizeof(ops_));
        ops_.destroy = ll_destroy;
        ops_.create = ll_create;
        ops_.release = ll_release;
        ops_.unlink = ll_unlink;
        ops_.forget_multi = ll_forget_multi;
        ops_.getattr = ll_getattr;
        ops_.lookup = ll_lookup;
        ops_.opendir = ll_opendir;
        ops_.readdir = ll_readdir;
        ops_.releasedir = ll_releasedir;
        ops_.open = ll_open;
        ops_.write_buf = ll_write_buf;
        ops_.read = ll_read;
        ops_.mkdir = ll_mkdir;
        ops_.rmdir = ll_rmdir;
        ops_.rename = ll_rename;
        ops_.setattr = ll_setattr;
        ops_.readlink = ll_readlink;
        ops_.symlink = ll_symlink;
        ops_.fsync = ll_fsync;
        ops_.fsyncdir = ll_fsyncdir;
        ops_.statfs = ll_statfs;
        ops_.link = ll_link;
        ops_.access = ll_access;
        ops_.mknod = ll_mknod;
        ops_.fallocate = ll_fallocate;
    }

    const fuse_lowlevel_ops& ops() const { return ops_; }

public:
    virtual void destroy() = 0;
    virtual int
    lookup(fuse_ino_t parent_ino, const std::string& name, struct stat* st)
      = 0;
    virtual void forget(fuse_ino_t ino, long unsigned nlookup) = 0;
    virtual int statfs(fuse_ino_t ino, struct statvfs* stbuf) = 0;

    virtual int mknod(
      fuse_ino_t parent_ino,
      const std::string& name,
      mode_t mode,
      dev_t rdev,
      struct stat* st,
      uid_t uid,
      gid_t gid)
      = 0;

    virtual int symlink(
      const std::string& link,
      fuse_ino_t parent_ino,
      const std::string& name,
      struct stat* st,
      uid_t uid,
      gid_t gid)
      = 0;

    virtual int link(
      fuse_ino_t ino,
      fuse_ino_t newparent_ino,
      const std::string& newname,
      struct stat* st,
      uid_t uid,
      gid_t gid)
      = 0;

    virtual int rename(
      fuse_ino_t parent_ino,
      const std::string& name,
      fuse_ino_t newparent_ino,
      const std::string& newname,
      uid_t uid,
      gid_t gid)
      = 0;

    virtual int
    unlink(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid)
      = 0;

    virtual int access(fuse_ino_t ino, int mask, uid_t uid, gid_t gid) = 0;

    virtual int getattr(fuse_ino_t ino, struct stat* st, uid_t uid, gid_t gid)
      = 0;

    virtual int setattr(
      fuse_ino_t ino,
      FileHandle* fh,
      struct stat* attr,
      int to_set,
      uid_t uid,
      gid_t gid)
      = 0;

    virtual ssize_t
    readlink(fuse_ino_t ino, char* path, size_t maxlen, uid_t uid, gid_t gid)
      = 0;

    virtual int mkdir(
      fuse_ino_t parent_ino,
      const std::string& name,
      mode_t mode,
      struct stat* st,
      uid_t uid,
      gid_t gid)
      = 0;

    virtual int opendir(fuse_ino_t ino, int flags, uid_t uid, gid_t gid) = 0;

    virtual ssize_t readdir(
      fuse_req_t req, fuse_ino_t ino, char* buf, size_t bufsize, off_t off)
      = 0;

    virtual int
    rmdir(fuse_ino_t parent_ino, const std::string& name, uid_t uid, gid_t gid)
      = 0;

    virtual void releasedir(fuse_ino_t ino) = 0;

    virtual int create(
      fuse_ino_t parent_ino,
      const std::string& name,
      mode_t mode,
      int flags,
      struct stat* st,
      FileHandle** fhp,
      uid_t uid,
      gid_t gid)
      = 0;

    virtual int
    open(fuse_ino_t ino, int flags, FileHandle** fhp, uid_t uid, gid_t gid)
      = 0;
    virtual ssize_t
    write_buf(FileHandle* fh, struct fuse_bufvec* bufv, off_t off)
      = 0;
    virtual ssize_t read(FileHandle* fh, off_t offset, size_t size, char* buf)
      = 0;
    virtual void release(fuse_ino_t ino, FileHandle* fh) = 0;

private:
    static filesystem_base* get(fuse_req_t req) {
        return get(fuse_req_userdata(req));
    }

    static filesystem_base* get(void* userdata) {
        return reinterpret_cast<filesystem_base*>(userdata);
    }

    static void ll_destroy(void* userdata) {
        auto fs = get(userdata);
        fs->destroy();
    }

    static void ll_create(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      mode_t mode,
      struct fuse_file_info* fi) {
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

    static void
    ll_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
        auto fs = get(req);
        auto fh = reinterpret_cast<FileHandle*>(fi->fh);

        fs->release(ino, fh);
        fuse_reply_err(req, 0);
    }

    static void ll_unlink(fuse_req_t req, fuse_ino_t parent, const char* name) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs->unlink(parent, name, ctx->uid, ctx->gid);
        fuse_reply_err(req, -ret);
    }

    static void
    ll_forget(fuse_req_t req, fuse_ino_t ino, long unsigned nlookup) {
        auto fs = get(req);

        fs->forget(ino, nlookup);
        fuse_reply_none(req);
    }

    static void ll_forget_multi(
      fuse_req_t req, size_t count, struct fuse_forget_data* forgets) {
        auto fs = get(req);

        for (size_t i = 0; i < count; i++) {
            const struct fuse_forget_data* f = forgets + i;
            fs->forget(f->ino, f->nlookup);
        }

        fuse_reply_none(req);
    }

    static void
    ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct stat st;
        int ret = fs->getattr(ino, &st, ctx->uid, ctx->gid);
        if (ret == 0)
            fuse_reply_attr(req, &st, ret);
        else
            fuse_reply_err(req, -ret);
    }

    static void ll_lookup(fuse_req_t req, fuse_ino_t parent, const char* name) {
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

    static void
    ll_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs->opendir(ino, fi->flags, ctx->uid, ctx->gid);
        if (ret == 0) {
            fuse_reply_open(req, fi);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    static void ll_readdir(
      fuse_req_t req,
      fuse_ino_t ino,
      size_t size,
      off_t off,
      struct fuse_file_info* fi) {
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

    static void
    ll_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
        auto fs = get(req);

        fs->releasedir(ino);
        fuse_reply_err(req, 0);
    }

    static void
    ll_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        // new files are handled by ll_create
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

    static void ll_write_buf(
      fuse_req_t req,
      fuse_ino_t ino,
      struct fuse_bufvec* bufv,
      off_t off,
      struct fuse_file_info* fi) {
        auto fs = get(req);
        auto fh = reinterpret_cast<FileHandle*>(fi->fh);

        ssize_t ret = fs->write_buf(fh, bufv, off);
        if (ret >= 0)
            fuse_reply_write(req, ret);
        else
            fuse_reply_err(req, -ret);
    }

    static void ll_read(
      fuse_req_t req,
      fuse_ino_t ino,
      size_t size,
      off_t off,
      struct fuse_file_info* fi) {
        auto fs = get(req);
        auto fh = reinterpret_cast<FileHandle*>(fi->fh);

        auto buf = std::unique_ptr<char[]>(new char[size]);

        ssize_t ret = fs->read(fh, off, size, buf.get());
        if (ret >= 0)
            fuse_reply_buf(req, buf.get(), ret);
        else
            fuse_reply_err(req, -ret);
    }

    static void
    ll_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode) {
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

    static void ll_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs->rmdir(parent, name, ctx->uid, ctx->gid);
        fuse_reply_err(req, -ret);
    }

    static void ll_rename(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      fuse_ino_t newparent,
      const char* newname) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs->rename(
          parent, name, newparent, newname, ctx->uid, ctx->gid);
        fuse_reply_err(req, -ret);
    }

    static void ll_setattr(
      fuse_req_t req,
      fuse_ino_t ino,
      struct stat* attr,
      int to_set,
      struct fuse_file_info* fi) {
        auto fs = get(req);
        auto fh = fi ? reinterpret_cast<FileHandle*>(fi->fh) : nullptr;
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs->setattr(ino, fh, attr, to_set, ctx->uid, ctx->gid);
        if (ret == 0)
            fuse_reply_attr(req, attr, 0);
        else
            fuse_reply_err(req, -ret);
    }

    static void ll_readlink(fuse_req_t req, fuse_ino_t ino) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);
        char path[PATH_MAX + 1];

        ssize_t ret = fs->readlink(
          ino, path, sizeof(path) - 1, ctx->uid, ctx->gid);
        if (ret >= 0) {
            path[ret] = '\0';
            fuse_reply_readlink(req, path);
        } else {
            int r = (int)ret;
            fuse_reply_err(req, -r);
        }
    }

    static void ll_symlink(
      fuse_req_t req, const char* link, fuse_ino_t parent, const char* name) {
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

    static void ll_fsync(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi) {
        fuse_reply_err(req, 0);
    }

    static void ll_fsyncdir(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi) {
        fuse_reply_err(req, 0);
    }

    static void ll_statfs(fuse_req_t req, fuse_ino_t ino) {
        auto fs = get(req);

        struct statvfs stbuf;
        std::memset(&stbuf, 0, sizeof(stbuf));

        int ret = fs->statfs(ino, &stbuf);
        if (ret == 0)
            fuse_reply_statfs(req, &stbuf);
        else
            fuse_reply_err(req, -ret);
    }

    static void ll_link(
      fuse_req_t req,
      fuse_ino_t ino,
      fuse_ino_t newparent,
      const char* newname) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct fuse_entry_param fe;
        std::memset(&fe, 0, sizeof(fe));

        int ret = fs->link(
          ino, newparent, newname, &fe.attr, ctx->uid, ctx->gid);
        if (ret == 0) {
            fe.ino = fe.attr.st_ino;
            fuse_reply_entry(req, &fe);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    static void ll_access(fuse_req_t req, fuse_ino_t ino, int mask) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs->access(ino, mask, ctx->uid, ctx->gid);
        fuse_reply_err(req, -ret);
    }

    static void ll_mknod(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      mode_t mode,
      dev_t rdev) {
        auto fs = get(req);
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct fuse_entry_param fe;
        std::memset(&fe, 0, sizeof(fe));

        int ret = fs->mknod(
          parent, name, mode, rdev, &fe.attr, ctx->uid, ctx->gid);
        if (ret == 0) {
            fe.ino = fe.attr.st_ino;
            fuse_reply_entry(req, &fe);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    static void ll_fallocate(
      fuse_req_t req,
      fuse_ino_t ino,
      int mode,
      off_t offset,
      off_t length,
      struct fuse_file_info* fi) {
        fuse_reply_err(req, 0);
    }

    fuse_lowlevel_ops ops_;
};
