#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "stddef.h"

#define BUFSIZE 1024

static int
is_sep(char c)
{
	return (c == ' ' || c == '-' || c == '\r' || c == '\t' ||
		c == '\n' || c == '.' || c == '/' || c == ',');
}

// Print a number in numbuf if it is divisible by 5 or 6.
static void
handle_number(char *numbuf, int nlen)
{
	if (nlen > 0) {
		numbuf[nlen] = '\0';
		int n = atoi(numbuf);
		if (n%5 == 0 || n%6 == 0) {
			printf("%d\n", n);
		}
	}
}

static void
scan_file(int fd)
{
	char readbuf[BUFSIZE];
	char numbuf[32];
	int nlen = 0;
	int can_start_number = 1;
	int n;

	while ((n = read(fd, readbuf, BUFSIZE)) > 0) {
		for (int i = 0; i < n; i++) {
			char c = readbuf[i];

			if (is_sep(c)) {
				handle_number(numbuf, nlen);
				nlen = 0;
				can_start_number = 1;
			} else if (c >= '0' && c <= '9') {
				if (can_start_number == 1 && nlen < sizeof(numbuf) - 1) {
					numbuf[nlen++] = c;
				}
			} else {
				nlen = 0;
				can_start_number = 0;
			}
		}
	}

	// We may have a number left in numbuf.
	handle_number(numbuf, nlen);
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(2, "Usage: sixfive FILE [FILE...]\n");
		exit(1);
	}

	int file_failures = 0;
	for (int f = 1; f < argc; f++) {
		int fd = open(argv[f], 0);
		if (fd < 0) {
			fprintf(2, "sixfive: cannot open %s\n", argv[f]);
			file_failures++;

			continue;
		}

		scan_file(fd);

		if (close(fd) != 0) {
			fprintf(2, "sixfive: problem closing %s\n", argv[f]);
			file_failures++;
		}
	}

	exit(file_failures);
}
