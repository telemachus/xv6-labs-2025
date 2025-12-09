#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define NULL ((void*)0)

int should_skip_entry(int inum, char *name) {
	return inum == 0 || strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

char *basename(char *path) {
	if (strcmp(path, ".") == 0 || (strcmp(path, "..") == 0) || (strcmp(path, "/") == 0)) {
		return path;
	}
	char *p;
	for (p = path + strlen(path); p >= path && *p != '/'; p--) {
		/* Do nothing loop. */;
	}

	return ++p;
}

int handle_match(char *filepath, int exec_argc, char **exec_cmd) {
	if (exec_cmd == NULL) {
		printf("%s\n", filepath);
		return 0;
	}

	char *exec_argv[MAXARG];
	int i;
	for (i = 0; i < exec_argc && i < MAXARG - 2; i++) {
		exec_argv[i] = exec_cmd[i];
	}
	exec_argv[i++] = filepath;
	exec_argv[i] = NULL;

	if (fork() == 0) {
		exec(exec_argv[0], exec_argv);
		fprintf(2, "find: -exec %s failed\n", exec_argv[0]);
		exit(EXIT_FAILURE);
	}

	int status = 0;
	wait(&status);
	if (status != 0) {
		fprintf(2, "find: -exec %s had an error\n", exec_argv[0]);
		return 1;
	}
	return 0;
}

int find(char *path, char *wanted, int exec_argc, char **exec_cmd) {
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
			has_error = handle_match(path, exec_argc, exec_cmd);
		}
		break;

	case T_DIR:
		if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
			close(fd);
			printf("find: path too long: %s\n", path);
			return EXIT_FAILURE;
		}

		strcpy(buf, path);
		p = buf + strlen(buf);
		*p++ = '/';
		while (read(fd, &de, sizeof(de)) == sizeof(de)) {
			if (should_skip_entry(de.inum, de.name)) {
				continue;
			}

			memmove(p, de.name, DIRSIZ);
			p[DIRSIZ] = 0;

			if (strcmp(wanted, de.name) == 0) {
				if (handle_match(buf, exec_argc, exec_cmd)) {
					has_error = 1;
				}
			}

			if (stat(buf, &st) < 0) {
				fprintf(2, "find: cannot stat %s\n", buf);
				has_error = 1;
			} else if (st.type == T_DIR) {
				if (find(buf, wanted, exec_argc, exec_cmd) != EXIT_SUCCESS) {
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

int main(int argc, char **argv) {
	int exec_argc = 0;
	char **exec_cmd = NULL;

	if (argc < 3) {
		fprintf(2, "Usage: find DIR NAME [-exec CMD [ARGS...]]\n");
		exit(EXIT_FAILURE);
	}
	if (argc > 3) {
		if (strcmp(argv[3], "-exec") != 0) {
			fprintf(2, "find: unrecognized argument: %s\n", argv[3]);
			fprintf(2, "Usage: find DIR NAME [-exec CMD [ARGS...]]\n");
			exit(EXIT_FAILURE);
		}
		exec_cmd = &argv[4];
		exec_argc = argc - 4;
	}

	exit(find(argv[1], argv[2], exec_argc, exec_cmd));
}
