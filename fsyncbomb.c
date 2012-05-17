/*
 * Copyright William D. Pijewski
 *
 * This program simulates a write-heaby database workload.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define	BLOCKSIZE	(4 * 1024)

boolean_t create_only = B_FALSE;
boolean_t fsync_after_write = B_FALSE;
boolean_t truncate_file = B_TRUE;
boolean_t quiet = B_FALSE;

uint64_t numfiles = 10;
ssize_t filesize = 10 * 1024 * 1024;

static ssize_t
fullwrite(int fd, const void *buf, size_t nbyte)
{
	ssize_t total = 0, ret;

	while (total < nbyte) {
		if ((ret = write(fd, buf + total, nbyte - total)) < 0)
			return (ret);
		total += ret;
	}

	return (total);
}

static int
dowrite(int fd, long nblocks, long blocksize)
{
	int ii, jj, kk;
	uint8_t value = 0;
	uint8_t *pattern;
	int patterns_per_block = (BLOCKSIZE / blocksize);

	if ((pattern = malloc(BLOCKSIZE)) == NULL)
		return (-1);

	/*
	 * Write a 8-bit pattern into the buffer, cycling through 0x00, 0x01,
	 * 0x02, ..., 0xff.
	 */
	for (ii = 0; ii < nblocks / patterns_per_block; ii++) {
		/*
		 * The pattern block size may be less than 128k, and if so,
		 * write multiple pattern blocks so we don't have to make a
		 * write(2) call for every pattern block.
		 */
		for (jj = 0; jj < patterns_per_block; jj++) {
			(void) memset(pattern + (jj * blocksize),
			    value++, blocksize);
		}

		if (fullwrite(fd, pattern, BLOCKSIZE) < BLOCKSIZE) {
			fprintf(stderr, "cannot write %d bytes: %s\n",
			    sizeof (pattern), strerror(errno));
			free(pattern);
			return (-1);
		}
	}

	free(pattern);
	return (0);
}

int
main(int argc, char *argv[])
{
	char file[MAXNAMELEN], c;
	int ii, jj, fd, oflags;

	extern int optind, optopt;
	extern char *optarg;

	errno = 0;

	while ((c = getopt(argc, argv, "cfn:qs:t")) != EOF) {
		switch (c) {
		case 'c':
			create_only = B_TRUE;
			break;
		case 'f':
			fsync_after_write = B_TRUE;
			break;
		case 'n':
			numfiles = atoi(optarg);
			break;
		case 'q':
			quiet = B_TRUE;
			break;
		case 's':
			filesize = atoi(optarg);
			break;
		case 't':
			truncate_file = B_TRUE;
			break;
		default:
			fprintf(stderr,
			    "Unrecognized option: -%c\n", optopt);
			return (-1);
		}
	}

	for (ii = 0; ii < numfiles; ii++) {
		(void) snprintf(file, sizeof (file), "./fsyncbomb.%d", ii);

		oflags = O_WRONLY | O_CREAT;
		if (truncate_file)
			oflags |= O_TRUNC;

		if ((fd = open(file, oflags)) < 0) {
			(void) fprintf(stderr, "cannot open file: %s",
			    strerror(errno));
			return (-1);
		}

		if (dowrite(fd, filesize / BLOCKSIZE, BLOCKSIZE) != 0) {
			(void) close(fd);
			return (-1);
		}

		if (!quiet) {
			(void) fprintf(stdout, "wrote %d bytes to %s\n",
			    filesize, file);
		}

		if (fsync_after_write) {
			if (fsync(fd) < 0) {
				(void) fprintf(stderr, "cannot fsync file: %s",
				    strerror(errno));
				(void) close(fd);
				return (-1);
			}

			if (!quiet)
				(void) fprintf(stdout, "fsync'd %s\n", file);
		}

		(void) close(fd);

		if (ii == numfiles - 1 && !create_only)
			ii = 0;
	}

	return (0);
}
