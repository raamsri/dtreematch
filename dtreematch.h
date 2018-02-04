#ifndef DTREEMATCH_H
#define DTREEMATCH_H

#include <glib.h>
#include <ftw.h>

#define PRINT_STDOUT(fmt, ...) \
                do { \
		    if (IS_VERBOSE) { \
			fprintf(stdout, fmt, __VA_ARGS__); \
			if (fflush(stdout)) perror("fflush"); \
		    } \
		} while(0)

#define PRINT_STDERR(fmt, ...) \
                do { \
		    if (IS_VERBOSE) { \
			fprintf(stderr, fmt, __VA_ARGS__); \
			if (fflush(stderr)) perror("fflush"); \
		    } \
		} while(0)


#define OPEN_FLAGS 	O_RDONLY 	| \
			O_NOFOLLOW 	| \
			O_NOATIME	| \
			O_LARGEFILE	| \
			O_NOCTTY	| \
			O_NONBLOCK


extern char     *PATH1;
extern char     *PATH2;
extern gint     MAX_LEVEL;
extern gchar    *HASH_TYPE;
extern gboolean IS_FOLLOW_SYMLINK;
extern gboolean IS_SKIP_CONTENT_HASH;
extern gboolean IS_SKIP_REFNAME;
extern gboolean IS_VERBOSE;
extern GChecksumType CHECKSUM_G;

gchar *get_file_checksum(const char *file_path, GChecksumType checksum_type_g);

int compare_path(const char *src_path, const struct stat *sbuf1, int type);

int dtree_check(const char *path, const struct stat *sbuf, int type, struct FTW *ftwb);

#endif
