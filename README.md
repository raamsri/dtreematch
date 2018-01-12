# dtreematch

__dtreematch__ compares the contents of two directory trees recursively to check if they are similar. The paths can also be mount points.
This is an attempt at quickly figuring out if the contents of two paths are not similar.

The match(comparison) happens in this order for an entry(file,dir) in the path:

  - File/dir pathname(availability/presence)
  - File type/format - regular file, directory, block, link, etc.
  - Symlinks' referent file name
  - Number of directory entries
  - File size
  - Checksum (for regular files only)

By default all of the above checks are performed. Few of these checks can be skipped by setting appropriate switches/flags.

If any of the above(selected) check fails for an entry while traversing(the very first mismatch), without continuing any further, the program bails out saying __'FAIL'__ along with the mismatch or program failure reason. If the contents are similar, exits with a string saying __'MATCH'__.

NOTE: Since we bail out on the very first mismatch, dtreematch can not give a full list of the differences between the paths and it's not meant for that(not yet).

```
Usage:
  dtreematch [OPTION...] "/mugiwara/lufy" "/mugiwara/zoro"

Help Options:
  -h, --help                Show help options

Application Options:
  -l, --max-level=N         Do not traverse tree beyond N level(s)
  -c, --checksum=md5        Valid hashing algorithms: md5, sha1, sha256, sha512.
  -F, --no-content-hash     Skip hash check for the contents of the file
  -S, --no-refname          Do not compare symbolic links' referent file path name
  -v, --verbose             Verbose output
```

Hasn't been tested for all cases yet. Suggestions, issues, remarks and any sort of contributions from you is most welcome.

#### Dependencies
GCC & GLib-2.0

#### To run the pre compiled binary, clone or fork this repo and `cd` to the directory:
`./dtreematch -v /ace/lufy/sabo /marimo/errocook/ussop`

#### To compile:
`gcc -Wall -Wextra -o dtreematch dtreematch.c $(pkg-config --cflags --libs glib-2.0)`

#### TODO:
  - Break it in to modules
  - Handle `strnxxx` of `string.h` stuff to avoid buffer overruns 
  - Checksum the first few bytes of the files before proceeding to do it for the entire file
  - Better exit/error reporting
  - Verbose output
  - Extend as a library
  - Support for SSH?
