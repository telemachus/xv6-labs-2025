#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

void memdump(char *fmt, char *data);

int
main(int argc, char *argv[])
{
	if(argc == 1){
		printf("Example 1:\n");
		int a[2] = { 61810, 2025 };
		memdump("ii", (char*) a);

		printf("Example 2:\n");
		memdump("S", "a string");

		printf("Example 3:\n");
		char *s = "another";
		memdump("s", (char *) &s);

		struct sss {
			char *ptr;
			int num1;
			short num2;
			char byte;
			char bytes[8];
		} example;

		example.ptr = "hello";
		example.num1 = 1819438967;
		example.num2 = 100;
		example.byte = 'z';
		strcpy(example.bytes, "xyzzy");

		printf("Example 4:\n");
		memdump("pihcS", (char*) &example);

		printf("Example 5:\n");
		memdump("sccccc", (char*) &example);
	} else if(argc == 2){
		// format in argv[1], up to 512 bytes of data from standard input.
		char data[512];
		int n = 0;
		memset(data, '\0', sizeof(data));
		while(n < sizeof(data)){
			int nn = read(0, data + n, sizeof(data) - n);
			if(nn <= 0)
				break;
			n += nn;
		}
		memdump(argv[1], data);
	} else {
		printf("Usage: memdump [format]\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

void
memdump(char *fmt, char *data)
{
	char c = '0';
	while ((c = *fmt++)) {
		if (c == 'i') {
			printf("%d\n", *(int *)data);
			data += sizeof(int);
		} else if (c == 'p') {
			printf("%lx\n", *(uint64 *)data);
			data += sizeof(uint64);
		} else if (c == 'h') {
			printf("%d\n", *(short*)data);
			data += sizeof(short);
		} else if (c == 'c') {
			printf("%c\n", *data);
			data += sizeof(char);
		} else if (c == 's') {
			printf("%s\n", *(char **)data);
			data += sizeof(char *);
		} else if (c == 'S') {
			printf("%s\n", data);
			break; 
		} else {
			fprintf(2, "memdump: bad format item: '%c'\n", c);
			exit(EXIT_FAILURE);
		}
	}
}
