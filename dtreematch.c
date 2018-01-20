#define _XOPEN_SOURCE 600
#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dtreematch.h"


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
                PRINT_STDERR("%s\n", file_path);
                perror("open");
                return NULL;
        }

        GChecksum *cksum_g = NULL;
        cksum_g = g_checksum_new(checksum_type_g);
        gchar *hash_str;
        if (cksum_g == NULL) {
                PRINT_STDERR("g_checksum_new returned NULL%s\n", "");
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


int compare_path(const char *src_path, const struct stat *sbuf1, int type)
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

        PRINT_STDOUT("[SRC:%s, TGT:%s]", src_path, tgt_path);

        if (lstat(tgt_path, &sbuf2) == -1) {
                perror("stat");
                PRINT_STDERR("File missing [SRC:%s, TGT:%s]\n", src_path, tgt_path);
                return -1;
        }

	if (IS_FOLLOW_SYMLINK) {
		if (!S_ISLNK(sbuf1->st_mode) && !S_ISLNK(sbuf2.st_mode)) {
			if (!((sbuf1->st_mode & S_IFMT) == (sbuf2.st_mode & S_IFMT))) {
				PRINT_STDERR("File format differs [SRC:%s, TGT:%s]\n", src_path, tgt_path);
				return -1;
			}
		}
	} else {
		if (!((sbuf1->st_mode & S_IFMT) == (sbuf2.st_mode & S_IFMT))) {
			PRINT_STDERR("File format differs [SRC:%s, TGT:%s]\n", src_path, tgt_path);
			return -1;
		}
	}

        if (type == FTW_D) {
                struct dirent **entrylist1;
                struct dirent **entrylist2;
                int nentry1;
                int nentry2;

                nentry1 = scandir(src_path, &entrylist1, NULL, alphasort);
                if (nentry1 < 0) {
                        perror("scandir");
                        return -1;
                }
                int n = nentry1;
                while (n--)
                        free(entrylist1[n]);
                free(entrylist1);

                nentry2 = scandir(tgt_path, &entrylist2, NULL, alphasort);
                if (nentry2 < 0) {
                        perror("scandir");
                        return -1;
                }
                n = nentry2;
                while (n--)
                        free(entrylist2[n]);
                free(entrylist2);


                if (nentry1 != nentry2) {
                        PRINT_STDERR("Number of directory entry differs [SRC:%s, TGT:%s]\n", src_path, tgt_path);
                        return -1;
                }
        } else if (type == FTW_F) {
                if (!(sbuf1->st_size == sbuf2.st_size)) {
                        PRINT_STDERR("File size differs [SRC:%s, TGT:%s]\n", src_path, tgt_path);
                        return -1;
                }

                if (!IS_SKIP_CONTENT_HASH && (S_ISREG(sbuf1->st_mode) && S_ISREG(sbuf2.st_mode))) {
                        char *cksum1 = NULL;
                        char *cksum2 = NULL;
                        cksum1 = (char *) get_file_checksum(src_path, CHECKSUM_G);
                        if (cksum1 == NULL)
                                return -1;
                        cksum2 = (char *) get_file_checksum(tgt_path, CHECKSUM_G);
                        if (cksum2 == NULL)
                                return -1;

                        if (strcmp(cksum1, cksum2)) {
                                PRINT_STDERR("File checksum differs [SRC:%s, TGT:%s]\n", src_path, tgt_path);
                                return -1;
                        }
		}
        } else if (!IS_SKIP_REFNAME && type == FTW_SL) {
                char *real_link1;
                char *real_link2;
                real_link1 = realpath(src_path, NULL);
                if (real_link1 == NULL) {
                        perror("realpath");
                        return -1;
                }
                real_link2 = realpath(tgt_path, NULL);
                if (real_link2 == NULL) {
                        perror("realpath");
                        return -1;
                }

		const char *rel_base1;
		const char *rel_base2;
		rel_base1 = real_link1 + strlen(PATH1);
		rel_base2 = real_link2 + strlen(PATH2);
		if (strcmp(rel_base1, rel_base2)) {
			if (strcmp(real_link1, real_link2)) {
				PRINT_STDERR("Symlinks resolve to different file names [SRC:%s, TGT:%s]\n", src_path, tgt_path);
				free(real_link1);
				free(real_link2);
				return -1;
			}
		}
		free(real_link1);
		free(real_link2);
        }

        free(tgt_path);
        PRINT_STDOUT(" - MATCH%s\n", "");
        return FTW_CONTINUE;
}


int dtree_check(const char *path, const struct stat *sbuf, int type,
                        struct FTW *ftwb)
{
        if (MAX_LEVEL != -1 && ftwb->level > MAX_LEVEL)
                return FTW_SKIP_SIBLINGS;
	switch(type) {
	case FTW_DNR:
	case FTW_NS:
                PRINT_STDERR("Could not read from path %s\n", path);
                return FTW_STOP;
	case FTW_D:
	case FTW_F:
	case FTW_SL:
                if (compare_path(path, sbuf, type))
                        return FTW_STOP;
		return FTW_CONTINUE;
	case FTW_SLN:
                PRINT_STDERR("File path is a symbolic link pointing to "
				"a nonexistent file %s. Skipping check!\n", path);
		return FTW_CONTINUE;
	default:
		PRINT_STDERR("Unexpected situation, exiting!%s\n", "");
		return FTW_STOP;
	}

}

int main(int argc, char *argv[])
{
        GOptionContext *argctx;
        GError *error_g = NULL;

        argctx = g_option_context_new("\"/mugiwara/lufy\" \"/mugiwara/zoro\"");
        g_option_context_add_main_entries(argctx, entries_g, NULL);
        g_option_context_set_description(argctx, "Please report bugs at https://github.com/six-k/dtreematch or ramsri.hp@gmail.com");

        if (!g_option_context_parse(argctx, &argc, &argv, &error_g)) {
                fprintf(stderr, "Failed parsing arguments: %s\n", error_g->message);
                exit(EXIT_FAILURE);
        }

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

        if (HASH_TYPE) {
                if (strcmp((char *)HASH_TYPE, "sha1") == 0) {
                        CHECKSUM_G = G_CHECKSUM_SHA1;
                } else if (strcmp((char *)HASH_TYPE, "sha256") == 0){
                        CHECKSUM_G = G_CHECKSUM_SHA256;
                } else if (strcmp((char *)HASH_TYPE, "sha512") == 0){
                        CHECKSUM_G = G_CHECKSUM_SHA512;
                } else {
                        CHECKSUM_G = G_CHECKSUM_MD5;
                        HASH_TYPE = strdup("md5");
                        if (HASH_TYPE == NULL) {
                                perror("strdup");
                                exit(EXIT_FAILURE);
                        }
                }
        } else {
                CHECKSUM_G = G_CHECKSUM_MD5;
                HASH_TYPE = strdup("md5");
                if (HASH_TYPE == NULL) {
                        perror("strdup");
                        exit(EXIT_FAILURE);
                }
        }

        PRINT_STDOUT(   "Source path:\t\t%s\n"    \
                        "Target path:\t\t%s\n"    \
                        "Hash type:\t\t%s\n"      \
                        "Max level:\t\t%s (%d)\n" \
                        "Follow symlinks?\t%s\n"\
                        "Hash contents?\t\t%s\n"  \
                        "Match refname?\t\t%s\n"  \
                        "\n",
                        PATH1,
                        PATH2,
                        (gchar *) HASH_TYPE,
                        (MAX_LEVEL == -1) ? "all" : "", MAX_LEVEL,
                        (IS_FOLLOW_SYMLINK) ? "Yes" : "No",
                        (!IS_SKIP_CONTENT_HASH) ? "Yes" : "No",
                        (!IS_SKIP_REFNAME) ? "Yes" : "No");



        int ftw_flags;
	if (IS_FOLLOW_SYMLINK) {
		ftw_flags       = FTW_ACTIONRETVAL;
	} else {
		ftw_flags       = FTW_PHYS      |
				/* FTW_MOUNT    | */
				FTW_ACTIONRETVAL;
	}

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
