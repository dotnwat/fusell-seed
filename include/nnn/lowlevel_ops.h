#pragma once
#include <concepts>
#include <fuse_lowlevel.h>
#include <type_traits>
#include <utility>

namespace foo {

// TODO init
template<typename T>
concept lowlevel_init = requires(T t) {
    {
        t.init(std::declval<void*>(), std::declval<struct fuse_conn_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_destroy = requires(T t) {
    { t.destroy(std::declval<void*>()) } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_lookup = requires(T t) {
    {
        t.lookup(fuse_req_t{}, fuse_ino_t{}, std::declval<const char*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_forget = requires(T t) {
    {
        t.forget(fuse_req_t{}, fuse_ino_t{}, std::declval<unsigned long>())
    } -> std::same_as<void>;
};
template<typename T>
concept lowlevel_get_attribute = requires(T t) {
    {
        t.getattr(
          fuse_req_t{}, fuse_ino_t{}, std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_set_attribute = requires(T t) {
    {
        t.setattr(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<struct stat*>(),
          int{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_read_symlink = requires(T t) {
    { t.readlink(fuse_req_t{}, fuse_ino_t{}) } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_make_node = requires(T t) {
    {
        t.mknod(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<const char*>(),
          mode_t{},
          dev_t{})
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_make_directory = requires(T t) {
    {
        t.mkdir(
          fuse_req_t{}, fuse_ino_t{}, std::declval<const char*>(), mode_t{})
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_unlink = requires(T t) {
    {
        t.unlink(fuse_req_t{}, fuse_ino_t{}, std::declval<const char*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_remove_directory = requires(T t) {
    {
        t.rmdir(fuse_req_t{}, fuse_ino_t{}, std::declval<const char*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_make_symlink = requires(T t) {
    {
        t.symlink(
          fuse_req_t{},
          std::declval<const char*>(),
          fuse_ino_t{},
          std::declval<const char*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_rename = requires(T t) {
    {
        t.rename(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<const char*>(),
          fuse_ino_t{},
          std::declval<const char*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_make_hard_link = requires(T t) {
    {
        t.link(
          fuse_req_t{}, fuse_ino_t{}, fuse_ino_t{}, std::declval<const char*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_open = requires(T t) {
    {
        t.open(
          fuse_req_t{}, fuse_ino_t{}, std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_read = requires(T t) {
    {
        t.read(
          fuse_req_t{},
          fuse_ino_t{},
          size_t{},
          off_t{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

// TODO write
template<typename T>
concept lowlevel_write = requires(T t) {
    {
        t.write(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<const char*>(),
          size_t{},
          off_t{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

// TODO flush
template<typename T>
concept lowlevel_flush = requires(T t) {
    {
        t.flush(
          fuse_req_t{}, fuse_ino_t{}, std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_release = requires(T t) {
    {
        t.release(
          fuse_req_t{}, fuse_ino_t{}, std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

// TODO fsync
template<typename T>
concept lowlevel_fsync = requires(T t) {
    {
        t.fsync(
          fuse_req_t{},
          fuse_ino_t{},
          int{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_open_directory = requires(T t) {
    {
        t.opendir(
          fuse_req_t{}, fuse_ino_t{}, std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_read_directory = requires(T t) {
    {
        t.readdir(
          fuse_req_t{},
          fuse_ino_t{},
          size_t{},
          off_t{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_release_directory = requires(T t) {
    {
        t.releasedir(
          fuse_req_t{}, fuse_ino_t{}, std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

// TODO fsyncdir
template<typename T>
concept lowlevel_fsyncdir = requires(T t) {
    {
        t.fsyncdir(
          fuse_req_t{},
          fuse_ino_t{},
          int{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_stat_filesystem = requires(T t) {
    { t.statfs(fuse_req_t{}, fuse_ino_t{}) } -> std::same_as<void>;
};

// TODO setxattr
template<typename T>
concept lowlevel_setxattr = requires(T t) {
    {
        t.setxattr(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<const char*>(),
          std::declval<const char*>(),
          size_t{},
          int{})
    } -> std::same_as<void>;
};

// TODO getxattr
template<typename T>
concept lowlevel_getxattr = requires(T t) {
    {
        t.getxattr(
          fuse_req_t{}, fuse_ino_t{}, std::declval<const char*>(), size_t{})
    } -> std::same_as<void>;
};

// TODO listxattr
template<typename T>
concept lowlevel_listxattr = requires(T t) {
    { t.listxattr(fuse_req_t{}, fuse_ino_t{}, size_t{}) } -> std::same_as<void>;
};

// TODO removexattr
template<typename T>
concept lowlevel_removexattr = requires(T t) {
    {
        t.removexattr(fuse_req_t{}, fuse_ino_t{}, std::declval<const char*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_access = requires(T t) {
    { t.access(fuse_req_t{}, fuse_ino_t{}, int{}) } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_create = requires(T t) {
    {
        t.create(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<const char*>(),
          mode_t{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

// TODO getlk
template<typename T>
concept lowlevel_getlk = requires(T t) {
    {
        t.getlk(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<struct fuse_file_info*>(),
          std::declval<struct flock*>())
    } -> std::same_as<void>;
};

// TODO setlk
template<typename T>
concept lowlevel_setlk = requires(T t) {
    {
        t.setlk(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<struct fuse_file_info*>(),
          std::declval<struct flock*>(),
          int{})
    } -> std::same_as<void>;
};

// TODO bmap
template<typename T>
concept lowlevel_bmap = requires(T t) {
    {
        t.bmap(fuse_req_t{}, fuse_ino_t{}, size_t{}, uint64_t{})
    } -> std::same_as<void>;
};

// TODO ioctl
template<typename T>
concept lowlevel_ioctl = requires(T t) {
    {
        t.ioctl(
          fuse_req_t{},
          fuse_ino_t{},
          int{},
          std::declval<void*>(),
          std::declval<struct fuse_file_info*>(),
          unsigned{},
          std::declval<const void*>(),
          size_t{},
          size_t{})
    } -> std::same_as<void>;
};

// TODO poll
template<typename T>
concept lowlevel_poll = requires(T t) {
    {
        t.poll(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<struct fuse_file_info*>(),
          std::declval<struct fuse_pollhandle*>())
    } -> std::same_as<void>;
};

template<typename T>
concept lowlevel_write_buffer = requires(T t) {
    {
        t.write_buf(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<struct fuse_bufvec*>(),
          off_t{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

// TODO retreive reply
template<typename T>
concept lowlevel_retrieve_reply = requires(T t) {
    {
        t.retrieve_reply(
          fuse_req_t{},
          std::declval<void*>(),
          fuse_ino_t{},
          off_t{},
          std::declval<struct fuse_bufvec*>())
    } -> std::same_as<void>;
};

// TODO forget_multi
template<typename T>
concept lowlevel_forget_multi = requires(T t) {
    {
        t.forget_multi(
          fuse_req_t{}, size_t{}, std::declval<struct fuse_forget_data*>())
    } -> std::same_as<void>;
};

// TODO flock
template<typename T>
concept lowlevel_flock = requires(T t) {
    {
        t.flock(
          fuse_req_t{},
          fuse_ino_t{},
          std::declval<struct fuse_file_info*>(),
          int{})
    } -> std::same_as<void>;
};

// TODO fallocate
template<typename T>
concept lowlevel_fallocate = requires(T t) {
    {
        t.fallocate(
          fuse_req_t{},
          fuse_ino_t{},
          int{},
          off_t{},
          off_t{},
          std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

template<typename T>
class lowlevel_ops {
public:
    lowlevel_ops(T& ll)
      : ll_(ll) {
        if constexpr (lowlevel_init<T>) {
            ops_.init = ll_init;
        }
        if constexpr (lowlevel_destroy<T>) {
            ops_.destroy = ll_destroy;
        }
        if constexpr (lowlevel_lookup<T>) {
            ops_.lookup = ll_lookup;
        }
        if constexpr (lowlevel_forget<T>) {
            ops_.forget = ll_forget;
        }
        if constexpr (lowlevel_get_attribute<T>) {
            ops_.getattr = ll_getattr;
        }
        if constexpr (lowlevel_set_attribute<T>) {
            ops_.setattr = ll_setattr;
        }
        if constexpr (lowlevel_read_symlink<T>) {
            ops_.readlink = ll_readlink;
        }
        if constexpr (lowlevel_make_node<T>) {
            ops_.mknod = ll_mknod;
        }
        if constexpr (lowlevel_make_directory<T>) {
            ops_.mkdir = ll_mkdir;
        }
        if constexpr (lowlevel_unlink<T>) {
            ops_.unlink = ll_unlink;
        }
        if constexpr (lowlevel_remove_directory<T>) {
            ops_.rmdir = ll_rmdir;
        }
        if constexpr (lowlevel_make_symlink<T>) {
            ops_.symlink = ll_symlink;
        }
        if constexpr (lowlevel_rename<T>) {
            ops_.rename = ll_rename;
        }
        if constexpr (lowlevel_make_hard_link<T>) {
            ops_.link = ll_link;
        }
        if constexpr (lowlevel_open<T>) {
            ops_.open = ll_open;
        }
        if constexpr (lowlevel_read<T>) {
            ops_.read = ll_read;
        }
        if constexpr (lowlevel_write<T>) {
            ops_.write = ll_write;
        }
        if constexpr (lowlevel_flush<T>) {
            ops_.flush = ll_flush;
        }
        if constexpr (lowlevel_release<T>) {
            ops_.release = ll_release;
        }
        if constexpr (lowlevel_fsync<T>) {
            ops_.fsync = ll_fsync;
        }
        if constexpr (lowlevel_open_directory<T>) {
            ops_.opendir = ll_opendir;
        }
        if constexpr (lowlevel_read_directory<T>) {
            ops_.readdir = ll_readdir;
        }
        if constexpr (lowlevel_release_directory<T>) {
            ops_.releasedir = ll_releasedir;
        }
        if constexpr (lowlevel_fsyncdir<T>) {
            ops_.fsyncdir = ll_fsyncdir;
        }
        if constexpr (lowlevel_stat_filesystem<T>) {
            ops_.statfs = ll_statfs;
        }
        if constexpr (lowlevel_setxattr<T>) {
            ops_.setxattr = ll_setxattr;
        }
        if constexpr (lowlevel_getxattr<T>) {
            ops_.getxattr = ll_getxattr;
        }
        if constexpr (lowlevel_listxattr<T>) {
            ops_.listxattr = ll_listxattr;
        }
        if constexpr (lowlevel_removexattr<T>) {
            ops_.removexattr = ll_removexattr;
        }
        if constexpr (lowlevel_access<T>) {
            ops_.access = ll_access;
        }
        if constexpr (lowlevel_create<T>) {
            ops_.create = ll_create;
        }
        if constexpr (lowlevel_getlk<T>) {
            ops_.getlk = ll_getlk;
        }
        if constexpr (lowlevel_setlk<T>) {
            ops_.setlk = ll_setlk;
        }
        if constexpr (lowlevel_bmap<T>) {
            ops_.bmap = ll_bmap;
        }
        if constexpr (lowlevel_ioctl<T>) {
            ops_.ioctl = ll_ioctl;
        }
        if constexpr (lowlevel_poll<T>) {
            ops_.poll = ll_poll;
        }
        if constexpr (lowlevel_write_buffer<T>) {
            ops_.write_buf = ll_write_buf;
        }
        if constexpr (lowlevel_retrieve_reply<T>) {
            ops_.retrieve_reply = ll_retrieve_reply;
        }
        if constexpr (lowlevel_forget_multi<T>) {
            ops_.forget_multi = ll_forget_multi;
        }
        if constexpr (lowlevel_flock<T>) {
            ops_.flock = ll_flock;
        }
        if constexpr (lowlevel_fallocate<T>) {
            ops_.fallocate = ll_fallocate;
        }
    }

    const fuse_lowlevel_ops& ops() const { return ops_; }
    void* userdata() { return &ll_; }

private:
    static T* get(void* userdata) { return static_cast<T*>(userdata); }
    static T* get(fuse_req_t req) { return get(fuse_req_userdata(req)); }

    static void ll_init(void* userdata, struct fuse_conn_info* conn)
        requires lowlevel_init<T>
    {
        get(userdata)->init(userdata, conn);
    }

    static void ll_destroy(void* userdata)
        requires lowlevel_destroy<T>
    {
        get(userdata)->destroy(userdata);
    }

    static void ll_lookup(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires lowlevel_lookup<T>
    {
        get(req)->lookup(req, parent, name);
    }

    static void ll_forget(fuse_req_t req, fuse_ino_t ino, long unsigned nlookup)
        requires lowlevel_forget<T>
    {
        get(req)->forget(req, ino, nlookup);
    }

    static void
    ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_get_attribute<T>
    {
        get(req)->getattr(req, ino, fi);
    }

    static void ll_setattr(
      fuse_req_t req,
      fuse_ino_t ino,
      struct stat* attr,
      int to_set,
      struct fuse_file_info* fi)
        requires lowlevel_set_attribute<T>
    {
        get(req)->setattr(req, ino, attr, to_set, fi);
    }

    static void ll_readlink(fuse_req_t req, fuse_ino_t ino)
        requires lowlevel_read_symlink<T>
    {
        get(req)->readlink(req, ino);
    }

    static void ll_mknod(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      mode_t mode,
      dev_t dev)
        requires lowlevel_make_node<T>
    {
        get(req)->mknod(req, parent, name, mode, dev);
    }

    static void
    ll_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode)
        requires lowlevel_make_directory<T>
    {
        get(req)->mkdir(req, parent, name, mode);
    }

    static void ll_unlink(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires lowlevel_unlink<T>
    {
        get(req)->unlink(req, parent, name);
    }

    static void ll_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires lowlevel_remove_directory<T>
    {
        get(req)->rmdir(req, parent, name);
    }

    static void ll_symlink(
      fuse_req_t req, const char* link, fuse_ino_t parent, const char* name)
        requires lowlevel_make_symlink<T>
    {
        get(req)->symlink(req, link, parent, name);
    }

    static void ll_rename(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      fuse_ino_t newparent,
      const char* newname)
        requires lowlevel_rename<T>
    {
        get(req)->rename(req, parent, name, newparent, newname);
    }

    static void ll_link(
      fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char* newname)
        requires lowlevel_make_hard_link<T>
    {
        get(req)->link(req, ino, newparent, newname);
    }

    static void
    ll_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_open<T>
    {
        get(req)->open(req, ino, fi);
    }

    static void ll_read(
      fuse_req_t req,
      fuse_ino_t ino,
      size_t size,
      off_t off,
      struct fuse_file_info* fi)
        requires lowlevel_read<T>
    {
        get(req)->read(req, ino, size, off, fi);
    }

    static void ll_write(
      fuse_req_t req,
      fuse_ino_t ino,
      const char* buf,
      size_t size,
      off_t off,
      struct fuse_file_info* fi)
        requires lowlevel_write<T>
    {
        get(req)->write(req, ino, buf, size, off, fi);
    }

    static void
    ll_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_flush<T>
    {
        get(req)->flush(req, ino, fi);
    }

    static void
    ll_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_release<T>
    {
        get(req)->release(req, ino, fi);
    }

    static void ll_fsync(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi)
        requires lowlevel_fsync<T>
    {
        get(req)->fsync(req, ino, datasync, fi);
    }

    static void
    ll_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_open_directory<T>
    {
        get(req)->opendir(req, ino, fi);
    }

    static void ll_readdir(
      fuse_req_t req,
      fuse_ino_t ino,
      size_t size,
      off_t off,
      struct fuse_file_info* fi)
        requires lowlevel_read_directory<T>
    {
        get(req)->readdir(req, ino, size, off, fi);
    }

    static void
    ll_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_release_directory<T>
    {
        get(req)->releasedir(req, ino, fi);
    }

    static void ll_fsyncdir(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi)
        requires lowlevel_fsyncdir<T>
    {
        get(req)->fsyncdir(req, ino, datasync, fi);
    }

    static void ll_statfs(fuse_req_t req, fuse_ino_t ino)
        requires lowlevel_stat_filesystem<T>
    {
        get(req)->statfs(req, ino);
    }

    static void ll_setxattr(
      fuse_req_t req,
      fuse_ino_t ino,
      const char* name,
      const char* value,
      size_t size,
      int flags)
        requires lowlevel_setxattr<T>
    {
        get(req)->setxattr(req, ino, name, value, size, flags);
    }

    static void
    ll_getxattr(fuse_req_t req, fuse_ino_t ino, const char* name, size_t size)
        requires lowlevel_getxattr<T>
    {
        get(req)->getxattr(req, ino, name, size);
    }

    static void ll_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size)
        requires lowlevel_listxattr<T>
    {
        get(req)->listxattr(req, ino, size);
    }

    static void ll_removexattr(fuse_req_t req, fuse_ino_t ino, const char* name)
        requires lowlevel_removexattr<T>
    {
        get(req)->removexattr(req, ino, name);
    }

    static void ll_access(fuse_req_t req, fuse_ino_t ino, int mask)
        requires lowlevel_access<T>
    {
        get(req)->access(req, ino, mask);
    }

    static void ll_create(
      fuse_req_t req,
      fuse_ino_t parent,
      const char* name,
      mode_t mode,
      struct fuse_file_info* fi)
        requires lowlevel_create<T>
    {
        get(req)->create(req, parent, name, mode, fi);
    }

    static void ll_getlk(
      fuse_req_t req,
      fuse_ino_t ino,
      struct fuse_file_info* fi,
      struct flock* lock)
        requires lowlevel_getlk<T>
    {
        get(req)->getlk(req, ino, fi, lock);
    }

    static void ll_setlk(
      fuse_req_t req,
      fuse_ino_t ino,
      struct fuse_file_info* fi,
      struct flock* lock,
      int sleep)
        requires lowlevel_setlk<T>
    {
        get(req)->setlk(req, ino, fi, lock, sleep);
    }

    static void
    ll_bmap(fuse_req_t req, fuse_ino_t ino, size_t blocksize, uint64_t idx)
        requires lowlevel_bmap<T>
    {
        get(req)->bmap(req, ino, blocksize, idx);
    }

    static void ll_ioctl(
      fuse_req_t req,
      fuse_ino_t ino,
      int cmd,
      void* arg,
      struct fuse_file_info* fi,
      unsigned flags,
      const void* in_buf,
      size_t in_bufsz,
      size_t out_bufsz)
        requires lowlevel_ioctl<T>
    {
        get(req)->ioctl(
          req, ino, cmd, arg, fi, flags, in_buf, in_bufsz, out_bufsz);
    }

    static void ll_poll(
      fuse_req_t req,
      fuse_ino_t ino,
      struct fuse_file_info* fi,
      struct fuse_pollhandle* ph)
        requires lowlevel_poll<T>
    {
        get(req)->poll(req, ino, fi, ph);
    }

    static void ll_write_buf(
      fuse_req_t req,
      fuse_ino_t ino,
      struct fuse_bufvec* bufv,
      off_t off,
      struct fuse_file_info* fi)
        requires lowlevel_write_buffer<T>
    {
        get(req)->write_buf(req, ino, bufv, off, fi);
    }

    static void ll_retrieve_reply(
      fuse_req_t req,
      void* cookie,
      fuse_ino_t ino,
      off_t offset,
      struct fuse_bufvec* bufv)
        requires lowlevel_retrieve_reply<T>
    {
        get(req)->retrieve_reply(req, cookie, ino, offset, bufv);
    }

    static void ll_forget_multi(
      fuse_req_t req, size_t count, struct fuse_forget_data* forgets)
        requires lowlevel_forget_multi<T>
    {
        get(req)->forget_multi(req, count, forgets);
    }

    static void
    ll_flock(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi, int op)
        requires lowlevel_flock<T>
    {
        get(req)->flock(req, ino, fi, op);
    }

    static void ll_fallocate(
      fuse_req_t req,
      fuse_ino_t ino,
      int mode,
      off_t offset,
      off_t length,
      struct fuse_file_info* fi)
        requires lowlevel_fallocate<T>
    {
        get(req)->fallocate(req, ino, mode, offset, length, fi);
    }

    T& ll_;
    fuse_lowlevel_ops ops_{};
};

} // namespace foo
