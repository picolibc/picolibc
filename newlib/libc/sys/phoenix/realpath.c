/* Written 2000 by Werner Almesberger */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static int resolve_path(char *path, char *result, char *pos)
{
	if (*path == '/') {
		*result = '/';
		pos = result+1;
		++path;
	}

	*pos = 0;
	if (!*path)
		return 0;

	while (1) {
		struct stat st;

		char *slash = *path ? strchr(path,'/') : NULL;
		if (slash)
			*slash = 0;

		if (!path[0] || (path[0] == '.' &&	(!path[1] || (path[1] == '.' && !path[2])))) {
			--pos;
			if (pos != result && path[0] && path[1])
				while (*--pos != '/');
		}
		else {
			strcpy(pos,path);
			if (lstat(result,&st) < 0)
				return -1;

			if (S_ISLNK(st.st_mode)) {
				char buf[PATH_MAX_SIZE];

				if (readlink(result,buf,sizeof(buf)) < 0)
					return -1;
				
				*pos = 0;
				if (slash) {
					*slash = '/';
					strcat(buf, slash);
				}
	
				strcpy(path,buf);
				if (*path == '/')
					result[1] = 0;

				pos = strchr(result,0);
				continue;
			}

			pos = strchr(result,0);
		}
	
		if (slash) {
	    	*pos++ = '/';
	    	path = slash + 1;
		}

		*pos = 0;
		if (!slash)
			break;
    }

	return 0;
}

char *realpath(const char *path, char *resolved_path)
{
	char cwd[PATH_MAX_SIZE];
	char *path_copy;
	int res;

	if (!*path) {
		errno = ENOENT;
		return NULL;
	}

	if (!getcwd(cwd, sizeof(cwd)))
		return NULL;
    
	strcpy(resolved_path, "/");
	if (resolve_path(cwd, resolved_path, resolved_path))
		return NULL;

	strcat(resolved_path, "/");
	path_copy = strdup(path);
	if (!path_copy)
		return NULL;

	res = resolve_path(path_copy, resolved_path, strchr(resolved_path, 0));
	free(path_copy);
	if (res)
		return NULL;

	return resolved_path;
}
