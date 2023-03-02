#pragma once
#include <concepts>
#include <fuse_lowlevel.h>
#include <string>
#include <type_traits>

struct FileHandle;

namespace foo {

template<typename T>
concept destroy = requires(T t) {
    { t.destroy() } -> std::same_as<void>;
};

template<typename T>
concept lookup = requires(T t) {
    {
        t.lookup(
          fuse_ino_t{},
          std::declval<const std::string&>(),
          std::declval<struct stat*>())
    } -> std::same_as<int>;
};

template<typename T>
concept forget = requires(T t) {
    {
        t.forget(fuse_ino_t{}, std::declval<long unsigned>())
    } -> std::same_as<void>;
};

template<typename T>
concept get_attribute = requires(T t) {
    {
        t.getattr(fuse_ino_t{}, std::declval<struct stat*>(), uid_t{}, gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept set_attribute = requires(T t) {
    {
        t.setattr(
          fuse_ino_t{},
          std::declval<FileHandle*>(),
          std::declval<struct stat*>(),
          int{},
          uid_t{},
          gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept read_symlink = requires(T t) {
    {
        t.readlink(
          fuse_ino_t{}, std::declval<char*>(), size_t{}, uid_t{}, gid_t{})
    } -> std::same_as<ssize_t>;
};

template<typename T>
concept make_node = requires(T t) {
    {
        t.mknod(
          fuse_ino_t{},
          std::declval<const std::string&>(),
          mode_t{},
          dev_t{},
          std::declval<struct stat*>(),
          uid_t{},
          gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept make_directory = requires(T t) {
    {
        t.mkdir(
          fuse_ino_t{},
          std::declval<const std::string&>(),
          mode_t{},
          std::declval<struct stat*>(),
          uid_t{},
          gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept unlink = requires(T t) {
    {
        t.unlink(
          fuse_ino_t{}, std::declval<const std::string&>(), uid_t{}, gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept remove_directory = requires(T t) {
    {
        t.rmdir(
          fuse_ino_t{}, std::declval<const std::string&>(), uid_t{}, gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept make_symlink = requires(T t) {
    {
        t.symlink(
          std::declval<const std::string&>(),
          fuse_ino_t{},
          std::declval<const std::string&>(),
          std::declval<struct stat*>(),
          uid_t{},
          gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept rename = requires(T t) {
    {
        t.rename(
          fuse_ino_t{},
          std::declval<const std::string&>(),
          fuse_ino_t{},
          std::declval<const std::string&>(),
          uid_t{},
          gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept make_hard_link = requires(T t) {
    {
        t.link(
          fuse_ino_t{},
          fuse_ino_t{},
          std::declval<const std::string&>(),
          std::declval<struct stat*>(),
          uid_t{},
          gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept open = requires(T t) {
    {
        t.open(
          fuse_ino_t{}, int{}, std::declval<FileHandle**>(), uid_t{}, gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept read = requires(T t) {
    {
        t.read(
          std::declval<FileHandle*>(), off_t{}, size_t{}, std::declval<char*>())
    } -> std::same_as<ssize_t>;
};

template<typename T>
concept release = requires(T t) {
    {
        t.release(fuse_ino_t{}, std::declval<FileHandle*>())
    } -> std::same_as<void>;
};

template<typename T>
concept open_directory = requires(T t) {
    { t.opendir(fuse_ino_t{}, int{}, uid_t{}, gid_t{}) } -> std::same_as<int>;
};

template<typename T>
concept read_directory = requires(T t) {
    {
        t.readdir(
          fuse_req_t{}, fuse_ino_t{}, std::declval<char*>(), size_t{}, off_t{})
    } -> std::same_as<ssize_t>;
};

template<typename T>
concept release_directory = requires(T t) {
    { t.releasedir(fuse_ino_t{}) } -> std::same_as<void>;
};

template<typename T>
concept stat_filesystem = requires(T t) {
    {
        t.statfs(fuse_ino_t{}, std::declval<struct statvfs*>())
    } -> std::same_as<int>;
};

template<typename T>
concept access = requires(T t) {
    { t.access(fuse_ino_t{}, int{}, uid_t{}, gid_t{}) } -> std::same_as<int>;
};

template<typename T>
concept create = requires(T t) {
    {
        t.create(
          fuse_ino_t{},
          std::declval<const std::string&>(),
          mode_t{},
          int{},
          std::declval<struct stat*>(),
          std::declval<FileHandle**>(),
          uid_t{},
          gid_t{})
    } -> std::same_as<int>;
};

template<typename T>
concept write_buffer = requires(T t) {
    {
        t.write_buf(
          std::declval<FileHandle*>(),
          std::declval<struct fuse_bufvec*>(),
          off_t{})
    } -> std::same_as<ssize_t>;
};

} // namespace foo
