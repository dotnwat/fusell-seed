# Low-level FUSE file system boilerplate

[![Build Status](https://travis-ci.org/noahdesu/fusell-seed.svg?branch=master)](https://travis-ci.org/noahdesu/fusell-seed)

This project is a bare bones implementation of an in-memory FUSE file system
that uses the low-level FUSE interface. It passes a large percentage of the
POSIX compliance test suite, is regularly stress tested with numerous
workloads, and can serve as a good starting point for your next file system
project.

Many FUSE file systems found out in the wild use the high-level FUSE interface
that operates on file paths. In contrast to the path-based interface, the
low-level interface in FUSE operates on i-nodes, and offers a lot of flexibility.
However, there seems to be relatively few examples of how to build a file system
with this more advanced interface.

# Testing Process

The testing we perform focuses on support across many distributions,
compliance to the POSIX standard, and stress testing with many workloads.
Continuous integration builds and mounts the file system for a variety of OS
distributions. In addition, the continuous integration process runs the Tuxera
POSIX test suite on Ubuntu Trusty.

A broader set of tests are periodically run (e.g. before new releases) that
exercise the file system using a variety of real-world workloads.
Interestingly, during development, a significant number of issues were
surfaced by running real applications that did not show up when using the
POSIX test suite.  This larger set of test is not run as part of the
continuous integration process because they either take too long or require
too much memory.

The set of tests can be found in the `test/` directory. A selection:

* Run the Tuxera POSIX test suite (test/posix.sh)
* Build the Linux kernel (test/kernel.sh)
* Build the Ceph storage system (test/ceph.sh)
* Multiple configurations of iozone (test/iozone.sh)
* Build PostgreSQL and run tests (test/postgres.sh)
* Build Git and run unit tests (test/git.sh)

# Notes

[nwatkins@kyoto fusell-seed]$ ./main mount -o fsname=fusell -o allow_other -o atomic_o_trunc
Heap size:             512
write interface: write_buf
fusermount: option allow_other only allowed if 'user_allow_other' is set in
/etc/fuse.conf
