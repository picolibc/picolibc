#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int isatty (int fd) {
	return 1;
}
