#pragma once
#include <cassert>
#include <cstring>
#include <fuse_lowlevel.h>
#include <memory>

#include "filesystem.h"
#include "lowlevel_ops.h"

namespace foo {

template<typename T>
class default_lowlevel_ops {
public:
    template<typename... Args>
    default_lowlevel_ops(Args&&... args)
      : fs_(std::forward<Args>(args)...) {}

    void destroy(void* userdata)
        requires destroy<T>
    {
        fs_.destroy();
    }

    void lookup(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires lookup<T>
    {
        struct fuse_entry_param fe;
        std::memset(&fe, 0, sizeof(fe));

        int ret = fs_.lookup(parent, name, &fe.attr);
        if (ret == 0) {
            fe.ino = fe.attr.st_ino;
            fe.generation = 0;
            fuse_reply_entry(req, &fe);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    void forget(fuse_req_t req, fuse_ino_t ino, long unsigned nlookup)
        requires forget<T>
    {
        fs_.forget(ino, nlookup);
        fuse_reply_none(req);
    }

    void getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires get_attribute<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct stat st;
        int ret = fs_.getattr(ino, &st, ctx->uid, ctx->gid);
        if (ret == 0)
            fuse_reply_attr(req, &st, ret);
        else
            fuse_reply_err(req, -ret);
    }

    void setattr(
      fuse_req_t req,
      fuse_ino_t ino,
      struct stat* attr,
      int to_set,
      struct fuse_file_info* fi)
        requires set_attribute<T>
    {
        auto fh = fi ? reinterpret_cast<FileHandle*>(fi->fh) : nullptr;
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs_.setattr(ino, fh, attr, to_set, ctx->uid, ctx->gid);
        if (ret == 0)
            fuse_reply_attr(req, attr, 0);
        else
            fuse_reply_err(req, -ret);
    }

    void readlink(fuse_req_t req, fuse_ino_t ino)
        requires read_symlink<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);
        char path[PATH_MAX + 1];

        ssize_t ret = fs_.readlink(
          ino, path, sizeof(path) - 1, ctx->uid, ctx->gid);
        if (ret >= 0) {
            path[ret] = '\0';
            fuse_reply_readlink(req, path);
        } else {
            int r = (int)ret;
            fuse_reply_err(req, -r);
        }
    }

    void mknod(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      mode_t mode,
      dev_t rdev)
        requires make_node<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct fuse_entry_param fe;
        std::memset(&fe, 0, sizeof(fe));

        int ret = fs_.mknod(
          parent, name, mode, rdev, &fe.attr, ctx->uid, ctx->gid);
        if (ret == 0) {
            fe.ino = fe.attr.st_ino;
            fuse_reply_entry(req, &fe);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    void mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode)
        requires make_directory<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct fuse_entry_param fe;
        std::memset(&fe, 0, sizeof(fe));

        int ret = fs_.mkdir(parent, name, mode, &fe.attr, ctx->uid, ctx->gid);
        if (ret == 0) {
            fe.ino = fe.attr.st_ino;
            fe.generation = 0;
            fe.entry_timeout = 1.0;
            fuse_reply_entry(req, &fe);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    void unlink(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires unlink<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs_.unlink(parent, name, ctx->uid, ctx->gid);
        fuse_reply_err(req, -ret);
    }

    void rmdir(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires remove_directory<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs_.rmdir(parent, name, ctx->uid, ctx->gid);
        fuse_reply_err(req, -ret);
    }

    void symlink(
      fuse_req_t req, const char* link, fuse_ino_t parent, const char* name)
        requires make_symlink<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct fuse_entry_param fe;
        std::memset(&fe, 0, sizeof(fe));

        int ret = fs_.symlink(link, parent, name, &fe.attr, ctx->uid, ctx->gid);
        if (ret == 0) {
            fe.ino = fe.attr.st_ino;
            fuse_reply_entry(req, &fe);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    void rename(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      fuse_ino_t newparent,
      const char* newname)
        requires rename<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs_.rename(
          parent, name, newparent, newname, ctx->uid, ctx->gid);
        fuse_reply_err(req, -ret);
    }

    void link(
      fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char* newname)
        requires make_hard_link<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct fuse_entry_param fe;
        std::memset(&fe, 0, sizeof(fe));

        int ret = fs_.link(
          ino, newparent, newname, &fe.attr, ctx->uid, ctx->gid);
        if (ret == 0) {
            fe.ino = fe.attr.st_ino;
            fuse_reply_entry(req, &fe);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    void open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires open<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        // new files are handled by create
        assert(!(fi->flags & O_CREAT));

        FileHandle* fh;
        int ret = fs_.open(ino, fi->flags, &fh, ctx->uid, ctx->gid);
        if (ret == 0) {
            fi->fh = reinterpret_cast<uint64_t>(fh);
            fuse_reply_open(req, fi);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    void read(
      fuse_req_t req,
      fuse_ino_t ino,
      size_t size,
      off_t off,
      struct fuse_file_info* fi)
        requires read<T>
    {
        auto fh = reinterpret_cast<FileHandle*>(fi->fh);

        auto buf = std::unique_ptr<char[]>(new char[size]);

        ssize_t ret = fs_.read(fh, off, size, buf.get());
        if (ret >= 0)
            fuse_reply_buf(req, buf.get(), ret);
        else
            fuse_reply_err(req, -ret);
    }

    void release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires release<T>
    {
        auto fh = reinterpret_cast<FileHandle*>(fi->fh);

        fs_.release(ino, fh);
        fuse_reply_err(req, 0);
    }

    // TODO
    void fsync(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi) {
        fuse_reply_err(req, 0);
    }

    void opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires open_directory<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs_.opendir(ino, fi->flags, ctx->uid, ctx->gid);
        if (ret == 0) {
            fuse_reply_open(req, fi);
        } else {
            fuse_reply_err(req, -ret);
        }
    }

    void readdir(
      fuse_req_t req,
      fuse_ino_t ino,
      size_t size,
      off_t off,
      struct fuse_file_info* fi)
        requires read_directory<T>
    {
        auto buf = std::unique_ptr<char[]>(new char[size]);

        ssize_t ret = fs_.readdir(req, ino, buf.get(), size, off);
        if (ret >= 0) {
            fuse_reply_buf(req, buf.get(), (size_t)ret);
        } else {
            int r = (int)ret;
            fuse_reply_err(req, -r);
        }
    }

    void releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires release_directory<T>
    {
        fs_.releasedir(ino);
        fuse_reply_err(req, 0);
    }

    // TODO
    void fsyncdir(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi) {
        fuse_reply_err(req, 0);
    }

    void statfs(fuse_req_t req, fuse_ino_t ino)
        requires stat_filesystem<T>
    {
        struct statvfs stbuf;
        std::memset(&stbuf, 0, sizeof(stbuf));

        int ret = fs_.statfs(ino, &stbuf);
        if (ret == 0)
            fuse_reply_statfs(req, &stbuf);
        else
            fuse_reply_err(req, -ret);
    }

    void access(fuse_req_t req, fuse_ino_t ino, int mask)
        requires access<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        int ret = fs_.access(ino, mask, ctx->uid, ctx->gid);
        fuse_reply_err(req, -ret);
    }

    void create(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      mode_t mode,
      struct fuse_file_info* fi)
        requires create<T>
    {
        const struct fuse_ctx* ctx = fuse_req_ctx(req);

        struct fuse_entry_param fe;
        std::memset(&fe, 0, sizeof(fe));

        FileHandle* fh;
        int ret = fs_.create(
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

    void write_buf(
      fuse_req_t req,
      fuse_ino_t ino,
      struct fuse_bufvec* bufv,
      off_t off,
      struct fuse_file_info* fi)
        requires write_buffer<T>
    {
        auto fh = reinterpret_cast<FileHandle*>(fi->fh);

        ssize_t ret = fs_.write_buf(fh, bufv, off);
        if (ret >= 0)
            fuse_reply_write(req, ret);
        else
            fuse_reply_err(req, -ret);
    }

    // TODO
    void forget_multi(
      fuse_req_t req, size_t count, struct fuse_forget_data* forgets) {
        for (size_t i = 0; i < count; i++) {
            const struct fuse_forget_data* f = forgets + i;
            fs_.forget(f->ino, f->nlookup);
        }

        fuse_reply_none(req);
    }

    // TODO
    void fallocate(
      fuse_req_t req,
      fuse_ino_t ino,
      int mode,
      off_t offset,
      off_t length,
      struct fuse_file_info* fi) {
        fuse_reply_err(req, 0);
    }

private:
    T fs_;
};

} // namespace foo
