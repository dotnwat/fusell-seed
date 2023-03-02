#pragma once
#include "default_lowlevel_ops.h"
#include "filesystem.h"
#include "lowlevel_ops.h"

namespace foo {

template<typename T, typename D = lowlevel_ops<default_lowlevel_ops<T>>>
class filesystem {
public:
    template<typename... Args>
    filesystem(Args&&... args)
      : fs_(std::make_unique<T>(std::forward<Args>(args)...))
      , ops_(std::make_unique<D>()) {}

    const fuse_lowlevel_ops& ops() const { return ops_->ops(); }
    T& userdata() { return *fs_; }

private:
    std::unique_ptr<T> fs_;
    std::unique_ptr<D> ops_;
};

} // namespace foo
