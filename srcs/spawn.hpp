#ifndef SPAWN_HPP
#define SPAWN_HPP
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void spawnProgram(char *pathToExec) {
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
}

#endif
