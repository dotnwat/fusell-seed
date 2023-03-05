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
      : ops_(std::forward<Args>(args)...) {}

    const fuse_lowlevel_ops& ops() const { return ops_.ops(); }
    T& userdata() { return ops_.userdata(); }

private:
    D ops_;
};

} // namespace foo
