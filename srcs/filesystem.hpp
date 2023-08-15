#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP
#include <stdio.h>
#include <ftw.h>
#include <unistd.h>
#include <limits.h>
#include <algorithm>
#include <string>
#if _WIN32
	#include <libloaderapi.h>
#elif __APPLE__
	#include <mach-o/dyld.h>
#endif
#if _WIN32
	#define SEP "\\"
#else
	#define SEP "/"
#endif

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

/**
 * Get absolute path of executable
*/
std::string executable_path() {
	std::string ret;
	char pBuf[PATH_MAX];
	int bytes = 0;
	size_t len = sizeof(pBuf); 

	#if _WIN32
		bytes = GetModuleFileName(NULL, pBuf, len);
	#elif __APPLE__
		bytes = _NSGetExecutablePath(pBuf, &len);
	#else
		bytes = std::min(readlink("/proc/self/exe", pBuf, len), static_cast<ssize_t>(len - 1));
		if (bytes >= 0)
			pBuf[bytes] = '\0';
	#endif
	if (bytes >= 0)
		ret.assign(pBuf);
	return ret;
}

/**
 * Get absolute path of dir where exectable is placed
*/
std::string executable_dir() {
	std::string ret = executable_path();
	size_t lastOcc = ret.find_last_of(SEP);

	ret.resize(lastOcc);
	return ret;
}

#endif
