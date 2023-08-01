#ifndef SPAWN_HPP
#define SPAWN_HPP

#include <iostream>
#include <string.h>
#include <string>
#if _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/types.h>
#endif

void spawnProgram(char *pathToExec) {
#if _WIN32
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	HANDLE hReadPipe, hWritePipe;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
		throw new std::runtime_error(std::string("pipe failed: ") + strerror(errno));

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;

	if (!CreateProcess(
		NULL, (LPSTR)pathToExec, NULL,
		NULL, TRUE, CREATE_NEW_CONSOLE,
		NULL, NULL, &si, &pi)
	)
		throw new std::runtime_error(std::string("CreateProcess failed: ") + strerror(errno));
	CloseHandle(hWritePipe);
#else
	pid_t pid;
	char filename[] = "empty" ;
	char * args[] = { filename, NULL };

	if ((pid = fork()) == -1)
		throw new std::exception();
	if (pid == 0) {
		execv(pathToExec, args);
		std::string err = std::string("spawn failed [");
		err.append(pathToExec);
		err.append("]: ");
		err.append(strerror(errno));
		throw new std::runtime_error(err);
	}
#endif
}
#endif
