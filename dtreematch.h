#ifndef DTREEMATCH_H
#define DTREEMATCH_H

#include <glib.h>
#include <ftw.h>

#define PRINT_STDOUT(fmt, ...) \
                do { if (IS_VERBOSE) fprintf(stdout, fmt, __VA_ARGS__); } \
                while(0)

#define PRINT_STDERR(fmt, ...) \
                do { if (IS_VERBOSE) fprintf(stderr, fmt, __VA_ARGS__); } \
                while(0)

static char     *PATH1;
static char     *PATH2;
static gint     MAX_LEVEL = -1;
static gchar    *HASH_TYPE;
static gboolean IS_VERBOSE = FALSE;
static gboolean IS_SKIP_CONTENT_HASH = FALSE;
static gboolean IS_SKIP_REFNAME = FALSE;
static GChecksumType CHECKSUM_G;


static GOptionEntry entries_g[] = {
        { "max-level", 'l', 0, G_OPTION_ARG_INT, &MAX_LEVEL, "Do not traverse tree beyond N level(s)", "N" },
        { "checksum", 'c', 0, G_OPTION_ARG_STRING, &HASH_TYPE, "Valid hashing algorithms: md5, sha1, sha256, sha512.", "md5" },
        { "no-content-hash", 'F', 0, G_OPTION_ARG_NONE, &IS_SKIP_CONTENT_HASH, "Skip hash check for the contents of the file", NULL },
        { "no-refname", 'S', 0, G_OPTION_ARG_NONE, &IS_SKIP_REFNAME, "Do not compare symbolic links' referent file path name", NULL },
        { "verbose", 'v', 0, G_OPTION_ARG_NONE, &IS_VERBOSE, "Verbose output", NULL },
        { NULL }
};

gchar *get_file_checksum(const char *file_path, GChecksumType checksum_type_g);

int compare_path(const char *src_path, const struct stat *sbuf1, int type);

int dtree_check(const char *path, const struct stat *sbuf, int type, struct FTW *ftwb);

#endif
