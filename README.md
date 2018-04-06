# file-finder
A faster alternative to find. It currently respects a gitignore in the directory being searched
by ommitting files that match lines in the gitignore.

At a high level, this spawns two threads - one that goes through the file system, enqueuing files from the
root and another that 

My primary motivation for building this was to be able to use it with [fzf](https://github.com/junegunn/fzf).
Specifically, with large projects, I wanted to limit the number of files to search through with fzf in vim.

# Installation
There are no required dependencies. Currently it can be compiled simply with:

`g++-7 -std=c++17 -O3 -Iff/include/ ff/ff.cpp`

Until `std::filesystem` is supported by g++, this will only work with POSIX systems, specfically those with
the `dirent.h` header (see `ff/include/ff/filesystem.h` for more details on the filesystem logic).

`ff` has been tested on `Mac OS X 10.12.6` and `Ubuntu 16.04`.

This is also a [buck](https://buckbuild.com/) project. You can install and use buck to compile the sources as well.

[gflags](https://github.com/gflags/gflags) is an optional dependency if you want support for flags,
but right now there are no implemented flags.

# TODOS (links to Github issues)
- [Use cmake](https://github.com/akshaynanavati/file-finder/issues/1)
- [Use std::filesystem](https://github.com/akshaynanavati/file-finder/issues/2)
- [Add option for a $HOME/.ffrc](https://github.com/akshaynanavati/file-finder/issues/3)
