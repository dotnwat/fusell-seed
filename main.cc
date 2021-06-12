#include <cassert>
#include <cstring>
#include <iostream>
#include <stddef.h>
#if defined(__linux__)
#include <linux/limits.h>
#elif defined(__APPLE__)
#include <sys/syslimits.h>
#endif

#include <fuse.h>
#include <fuse_lowlevel.h>
#include <fuse_opt.h>

#include "filesystem.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

enum {
    KEY_HELP,
};

struct filesystem_opts {
    size_t size;
    bool debug;
};

#define FS_OPT(t, p, v)                                                        \
    { t, offsetof(struct filesystem_opts, p), v }

static struct fuse_opt fs_fuse_opts[] = {
  FS_OPT("size=%llu", size, 0),
  FS_OPT("-debug", debug, 1),
  FUSE_OPT_KEY("-h", KEY_HELP),
  FUSE_OPT_KEY("--help", KEY_HELP),
  FUSE_OPT_END};

static void usage(const char* progname) {
    printf("file system options:\n"
           "    -o size=N          max file system size (bytes)\n"
           "    -debug             turn on verbose logging\n");
}

static int
fs_opt_proc(void* data, const char* arg, int key, struct fuse_args* outargs) {
    switch (key) {
    case FUSE_OPT_KEY_OPT:
        return 1;
    case FUSE_OPT_KEY_NONOPT:
        return 1;
    case KEY_HELP:
        usage(NULL);
        exit(1);
    default:
        assert(0);
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    struct filesystem_opts opts;

    // option defaults
    opts.size = 512 << 20;
    opts.debug = false;

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    if (fuse_opt_parse(&args, &opts, fs_fuse_opts, fs_opt_proc) == -1) {
        exit(1);
    }

    auto console = spdlog::stdout_color_mt("console");
    if (opts.debug) {
        console->set_level(spdlog::level::debug);
    } else {
        console->set_level(spdlog::level::info);
    }

    assert(opts.size > 0);

    struct fuse_chan* ch;
    int err = -1;

    FileSystem fs(opts.size, console);

    char* mountpoint = nullptr;
    if (
      fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1
      && (ch = fuse_mount(mountpoint, &args)) != NULL) {
        struct fuse_session* se;

        se = fuse_lowlevel_new(&args, &fs.ops(), sizeof(fs.ops()), &fs);
        if (se != NULL) {
            if (fuse_set_signal_handlers(se) != -1) {
                fuse_session_add_chan(se, ch);
                err = fuse_session_loop_mt(se);
                fuse_remove_signal_handlers(se);
                fuse_session_remove_chan(ch);
            }
            fuse_session_destroy(se);
        }
        fuse_unmount(mountpoint, ch);
    }
    fuse_opt_free_args(&args);
    if (mountpoint) {
        free(mountpoint);
    }

    int rv = err ? 1 : 0;

    return rv;
}
