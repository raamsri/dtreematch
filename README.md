# dtreematch (WIP: prototype stage)

__dtreematch__ compares the contents of two directory trees recursively to check if they are similar. The paths can also be mount points.
This is an attempt at quickly figuring out if the contents of two paths are not similar.

The match(comparison) happens in this order for an entry(file,dir) in the path:

  - File/dir pathname check - file/dir availability
  - File type/format check - regular file, directory, block, link, etc.
  - Number of directory entries
  - File size check
  - Checksum check (for regular files only)

If any of the above check fails for an entry while traversing(the very first mismatch), without continuing any further, the program bails out saying __'FAIL'__ along with the mismatch or program failure reason. If the contents are similar, exits with a string saying __'MATCH'__.

NOTE: Since we bail out on the very first mismatch, dtreematch can not give a full list of the differences between the paths and it's not meant for that(not yet).

The proto works but hasn't been tested for all cases yet. Suggestions, issues, remarks and any sort of contributions from you is most welcome. 

#### Dependencies
GCC & GLib-2.0

#### To run the pre compiled binary, clone or fork this repo and `cd` to the directory:
`./dtreematch /path/to/src /path/to/target`

#### To compile:
`gcc -Wall -Wextra -o dtreematch dtreematch.c $(pkg-config --cflags --libs glib-2.0)`

#### TODO:
  - Break it in to modules
  - Multithread/fork with IPC or singals
  - Handle `strnxxx` of `string.h` stuff to avoid buffer overruns 
  - Have input flags(switches) for configuring the comparison/check criteria
  - Checksum the first few bytes of the files before proceeding to do it for the entire file
  - Better exit/error reporting
  - Verbose output
  - Extend as a library
  - Make it portable?
  - Support for SSH?
