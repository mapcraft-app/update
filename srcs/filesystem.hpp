#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP
#include <stdio.h>
#include <ftw.h>
#include <unistd.h>
#include <limits.h>
#include <string>

int unlinkCb(
	const char *path,
	const struct stat *s,
	int typeFlag,
	struct FTW *ft
) {
	static_cast<void>(s);
	static_cast<void>(typeFlag);
	static_cast<void>(ft);
	int rm = remove(path);

	if (rm)
		perror(path);
	return rm;
}

/**
 * rm -rf
*/
int rm(char *path) {
	return nftw(path, unlinkCb, 64, FTW_DEPTH | FTW_PHYS);
}

/**
 * Get absolute current path
*/
std::string current_path() {
	std::string ret;
	char cwd[PATH_MAX];

	memset(cwd, 0, sizeof(cwd));
	if (!getcwd(cwd, sizeof(cwd)))
		perror(cwd);
	ret.assign(cwd);
	return ret;
}

#endif
