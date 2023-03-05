#pragma once
#include <concepts>
#include <fuse_lowlevel.h>
#include <type_traits>
#include <utility>

namespace foo {

// TODO init

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
// TODO flush

template<typename T>
concept lowlevel_release = requires(T t) {
    {
        t.release(
          fuse_req_t{}, fuse_ino_t{}, std::declval<struct fuse_file_info*>())
    } -> std::same_as<void>;
};

// TODO fsync

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
concept lowlevel_stat_filesystem = requires(T t) {
    { t.statfs(fuse_req_t{}, fuse_ino_t{}) } -> std::same_as<void>;
};

// TODO setxattr
// TODO getxattr
// TODO listxattr
// TODO removexattr

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
// TODO setlk
// TODO bmap
// TODO ioctl
// TODO poll

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
// TODO forget_multi
// TODO flock
// TODO fallocate

template<typename T>
class lowlevel_ops {
public:
    lowlevel_ops(T& ll)
      : ll_(ll) {
        if constexpr (lowlevel_destroy<T>) {
            ops_.destroy = ll_destroy;
        }
        if constexpr (lowlevel_lookup<T>) {
            ops_.lookup = ll_lookup;
        }
        if constexpr (lowlevel_forget<T>) {
            ops_.forget_multi = ll_forget_multi;
        }
        if constexpr (lowlevel_stat_filesystem<T>) {
            ops_.statfs = ll_statfs;
        }
        if constexpr (lowlevel_make_node<T>) {
            ops_.mknod = ll_mknod;
        }
        if constexpr (lowlevel_make_symlink<T>) {
            ops_.symlink = ll_symlink;
        }
        if constexpr (lowlevel_make_hard_link<T>) {
            ops_.link = ll_link;
        }
        if constexpr (lowlevel_rename<T>) {
            ops_.rename = ll_rename;
        }
        if constexpr (lowlevel_unlink<T>) {
            ops_.unlink = ll_unlink;
        }
        if constexpr (lowlevel_access<T>) {
            ops_.access = ll_access;
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
        if constexpr (lowlevel_make_directory<T>) {
            ops_.mkdir = ll_mkdir;
        }
        if constexpr (lowlevel_open_directory<T>) {
            ops_.opendir = ll_opendir;
        }
        if constexpr (lowlevel_read_directory<T>) {
            ops_.readdir = ll_readdir;
        }
        if constexpr (lowlevel_remove_directory<T>) {
            ops_.rmdir = ll_rmdir;
        }
        if constexpr (lowlevel_release_directory<T>) {
            ops_.releasedir = ll_releasedir;
        }
        if constexpr (lowlevel_create<T>) {
            ops_.create = ll_create;
        }
        if constexpr (lowlevel_open<T>) {
            ops_.open = ll_open;
        }
        if constexpr (lowlevel_write_buffer<T>) {
            ops_.write_buf = ll_write_buf;
        }
        if constexpr (lowlevel_read<T>) {
            ops_.read = ll_read;
        }
        if constexpr (lowlevel_release<T>) {
            ops_.release = ll_release;
        }

        ops_.fsync = ll_fsync;
        ops_.fsyncdir = ll_fsyncdir;
        ops_.fallocate = ll_fallocate;
    }

    const fuse_lowlevel_ops& ops() const { return ops_; }
    void* userdata() { return &ll_; }

private:
    static T* get(void* userdata) { return static_cast<T*>(userdata); }
    static T* get(fuse_req_t req) { return get(fuse_req_userdata(req)); }

    static void ll_destroy(void* userdata)
        requires lowlevel_destroy<T>
    {
        get(userdata)->destroy(userdata);
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

    static void
    ll_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_release<T>
    {
        get(req)->release(req, ino, fi);
    }

    static void ll_unlink(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires lowlevel_unlink<T>
    {
        get(req)->unlink(req, parent, name);
    }

    static void ll_forget(fuse_req_t req, fuse_ino_t ino, long unsigned nlookup)
        requires lowlevel_forget<T>
    {
        get(req)->forget(req, ino, nlookup);
    }

    // TODO
    static void ll_forget_multi(
      fuse_req_t req, size_t count, struct fuse_forget_data* forgets) {
        get(req)->forget_multi(req, count, forgets);
    }

    static void
    ll_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_get_attribute<T>
    {
        get(req)->getattr(req, ino, fi);
    }

    static void ll_lookup(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires lowlevel_lookup<T>
    {
        get(req)->lookup(req, parent, name);
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

    static void
    ll_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi)
        requires lowlevel_open<T>
    {
        get(req)->open(req, ino, fi);
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

    static void
    ll_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode)
        requires lowlevel_make_directory<T>
    {
        get(req)->mkdir(req, parent, name, mode);
    }

    static void ll_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name)
        requires lowlevel_remove_directory<T>
    {
        get(req)->rmdir(req, parent, name);
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

    static void ll_symlink(
      fuse_req_t req, const char* link, fuse_ino_t parent, const char* name)
        requires lowlevel_make_symlink<T>
    {
        get(req)->symlink(req, link, parent, name);
    }

    // TODO
    static void ll_fsync(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi) {
        get(req)->fsync(req, ino, datasync, fi);
    }

    // TODO
    static void ll_fsyncdir(
      fuse_req_t req, fuse_ino_t ino, int datasync, struct fuse_file_info* fi) {
        get(req)->fsyncdir(req, ino, datasync, fi);
    }

    static void ll_statfs(fuse_req_t req, fuse_ino_t ino)
        requires lowlevel_stat_filesystem<T>
    {
        get(req)->statfs(req, ino);
    }

    static void ll_link(
      fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char* newname)
        requires lowlevel_make_hard_link<T>
    {
        get(req)->link(req, ino, newparent, newname);
    }

    static void ll_access(fuse_req_t req, fuse_ino_t ino, int mask)
        requires lowlevel_access<T>
    {
        get(req)->access(req, ino, mask);
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

    // TODO
    static void ll_fallocate(
      fuse_req_t req,
      fuse_ino_t ino,
      int mode,
      off_t offset,
      off_t length,
      struct fuse_file_info* fi) {
        get(req)->fallocate(req, ino, mode, offset, length, fi);
    }

    T& ll_;
    fuse_lowlevel_ops ops_{};
};

} // namespace foo
