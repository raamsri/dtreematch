#define _XOPEN_SOURCE 600
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <ftw.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>

#define CHECKSUM_G G_CHECKSUM_MD5

static char *PATH1;
static char *PATH2;

gchar *get_file_checksum(const char *file_path, GChecksumType checksum_type_g)
{
        int rfd;
        int open_flags;
        ssize_t snr;
        char rbuf[4096];

        open_flags = 	O_RDONLY 	|
                        O_NOFOLLOW 	|
                        O_NOATIME	|
                        O_LARGEFILE	|
                        O_NOCTTY	|
                        O_NONBLOCK;
        rfd = open(file_path, open_flags);
        if (rfd == -1) {
                fprintf(stderr, "%s\n", file_path);
                perror("open");
                return NULL;
        }

        GChecksum *cksum_g = NULL;
        cksum_g = g_checksum_new(checksum_type_g);
        gchar *hash_str;
        if (cksum_g == NULL) {
                fprintf(stderr, "g_checksum_new returned NULL\n");
                return NULL;
        }

        while (pread(rfd, NULL, (size_t) 1, lseek(rfd, 0, SEEK_CUR))) {
                if ((snr = read(rfd, &rbuf, (ssize_t) 4096)) != -1) {
                        if (snr == 0)
                                break;
                        g_checksum_update(cksum_g, (guchar *) rbuf, (gssize) snr);
                } else {
                        perror("read");
                        return NULL;
                }
        }
        hash_str = g_strdup(g_checksum_get_string(cksum_g));
        close(rfd);
        g_checksum_free(cksum_g);

        return hash_str;
}

static int compare_path(const char *src_path, const struct stat *sbuf1, int type)
{
        struct stat sbuf2;
        const char *diff_path;
        char *tgt_path;

        diff_path = src_path + strlen(PATH1);
        tgt_path = calloc(1, strlen(PATH2) + strlen(diff_path) + 1);
        if (tgt_path == NULL) {
                perror("calloc");
                return -1;
        }

        tgt_path = strncpy(tgt_path, PATH2, strlen(PATH2) + 1);
        tgt_path = strncat(tgt_path, diff_path, strlen(diff_path));

        if (lstat(tgt_path, &sbuf2) == -1) {
                perror("stat");
                fprintf(stderr, "FAIL: file missing [%s, %s]\n", src_path, tgt_path);
                return -1;
        }

        if (!((sbuf1->st_mode & S_IFMT) == (sbuf2.st_mode & S_IFMT))) {
                fprintf(stderr, "FAIL: file format differs [%s, %s]\n", src_path, tgt_path);
                return -1;
        }
        if (type == FTW_F) {
                if (!(sbuf1->st_size == sbuf2.st_size)) {
                        fprintf(stderr, "FAIL: file size differs [%s, %s]\n", src_path, tgt_path);
                        return -1;
                }

                if (S_ISREG(sbuf1->st_mode) && S_ISREG(sbuf2.st_mode)) {
                        char *cksum1 = NULL;
                        char *cksum2 = NULL;
                        cksum1 = (char *) get_file_checksum(src_path, CHECKSUM_G);
                        if (cksum1 == NULL)
                                return -1;
                        cksum2 = (char *) get_file_checksum(tgt_path, CHECKSUM_G);
                        if (cksum2 == NULL)
                                return -1;

                        if (strcmp(cksum1, cksum2)) {
                                fprintf(stderr, "FAIL: file checksum differs [%s, %s]\n", src_path, tgt_path);
                                return -1;
                        }
		}
        }
        free(tgt_path);
        return 0;
}

static int dtree_check(const char *path, const struct stat *sbuf, int type,
                        struct FTW *ftwb)
{
	switch(type) {
	case FTW_D:
		if (ftwb->level == 0)
			return FTW_CONTINUE;
	case FTW_F:
	case FTW_SL:
                /*
                fprintf(stdout, "%-14ld::%-4ld::%-40s::%-14jd::%d::%s\n",
				(long) sbuf->st_ino,
				(long) sbuf->st_nlink,
				path,
				(intmax_t) sbuf->st_size,
				ftwb->level,
				path + ftwb->base);
                */
                if (compare_path(path, sbuf, type))
                        return FTW_STOP;
		return FTW_CONTINUE;
	default:
		fprintf(stderr, "Unexpected situation, exiting!\n");
		return FTW_STOP;
	}

}

int main(int argc, char *argv[])
{
	PATH1 = realpath((argc > 1) ? argv[1] : ".", NULL);
	if (PATH1 == NULL) {
		perror("realpath");
		exit(EXIT_FAILURE);
	}
	PATH2 = realpath((argc > 2) ? argv[2] : ".", NULL);
	if (PATH2 == NULL) {
		perror("realpath");
		exit(EXIT_FAILURE);
	}

        int ftw_flags;
        ftw_flags               = FTW_PHYS  |
                                  /* FTW_MOUNT | */
                                  FTW_ACTIONRETVAL;

        int nftw_ret;
        nftw_ret = nftw(PATH1, dtree_check, 30, ftw_flags);
	if (nftw_ret == -1) {
		perror("nftw");
		exit(EXIT_FAILURE);
	} else if (nftw_ret == FTW_STOP) {
                fprintf(stdout, "FAIL\n");
		exit(EXIT_FAILURE);
	} else if (nftw_ret == 0) {
                fprintf(stdout, "MATCH\n");
	}

        free(PATH1);
        free(PATH2);

        exit(EXIT_SUCCESS);
}
