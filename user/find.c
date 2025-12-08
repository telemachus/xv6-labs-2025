#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int
should_skip_entry(int inum, char *name)
{
	return inum == 0 || strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

char *basename(char *path)
{
	if (strcmp(path, ".") == 0 ||
	   (strcmp(path, "..") == 0) ||
	   (strcmp(path, "/") == 0)) {
		return path;
	}
	char *p;
	for (p = path + strlen(path); p >= path && *p != '/'; p--) {
		;
	}

	return ++p;
}

int
find(char *path, char *wanted)
{
	char buf[512], *p;
	int fd;
	int has_error = 0;
	struct dirent de;
	struct stat st;

	if ((fd = open(path, O_RDONLY)) < 0) {
		fprintf(2, "find: cannot open %s\n", path);
		return EXIT_FAILURE;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return EXIT_FAILURE;
	}

	switch (st.type) {
	case T_DEVICE:
	case T_FILE:
		if (strcmp(wanted, basename(path)) == 0) {
			printf("%s\n", path);
		}
		break;

	case T_DIR:
		if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
			printf("find: path too long: %s\n", path);
			return EXIT_FAILURE;
		}

		strcpy(buf, path);
		p = buf+strlen(buf);
		*p++ = '/';
		while (read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (should_skip_entry(de.inum, de.name) == 1) {
				continue;
			}

			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;
			if (strcmp(wanted, de.name) == 0) {
				printf("%s\n", buf);
			}
			if (stat(buf, &st) < 0) {
				fprintf(2, "find: cannot stat %s\n", buf);
				has_error = 1;
			} else if (st.type == T_DIR) {
				if (find(buf, wanted) != EXIT_SUCCESS) {
					has_error = 1;
				}
			}
		}
		break;
	}

	if (close(fd) != 0) {
		fprintf(2, "find: problem closing %s\n", path);
		return EXIT_FAILURE;
	}

	return has_error ? EXIT_FAILURE : EXIT_SUCCESS;
}

int
main(int argc, char *argv[])
{
	if(argc < 3){
		fprintf(2, "Usage: find DIR NAME\n");
		exit(EXIT_FAILURE);
	}

	exit(find(argv[1], argv[2]));
}
