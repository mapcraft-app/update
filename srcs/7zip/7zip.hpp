#ifndef SEVENZIP_HPP
#define SEVENZIP_HPP
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define PLATFORM TOSTRING(TARGET)
#include <iostream>
#include <cstring>
#include <sstream>
#include <regex>
#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "trim.hpp"
#include "filesystem.hpp"
#if _WIN32
	#include <windows.h>
#endif

typedef std::map<std::string, std::string> SevenZipList;
typedef std::vector<std::string> SevenZipVector;

bool isEmpty (const std::string &s) {
	return s.empty();
}

class SevenZip {
	std::regex percentReg;
	std::string stdOut;
	int percent;

	public:
		SevenZip() {
			this->percentReg = std::regex("(\\d+)%", std::regex::ECMAScript);
			this->stdOut = std::string("");
			this->percent = 0;
		}
		~SevenZip() {}
	private:
		std::string convertPlatform() {
			if (strcmp(PLATFORM, "windows") == 0)
				return "win";
			if (strcmp(PLATFORM, "macos") == 0)
				return "mac";
			return "unix";
		};

		std::string exec7z() {
			if (strcmp(PLATFORM, "windows") == 0)
				return "7za.exe";
			return "7za";
		}

		std::string type() {
			#if INTPTR_MAX == INT64_MAX // Is x64
				#if __arm__
					return "arm64";
				#else
					return "x64";
				#endif
			#else
				#if __arm__
					return "arm";
				#else
					return "ia32";
				#endif
			#endif
			return "x64";
		}

		std::string addToPath(std::string s) {
			return SEP + s;
		}

		std::string path7z() {
			std::string path = executable_dir();

			path += this->addToPath("srcs");
			path += this->addToPath("7zip");
			path += this->addToPath(this->convertPlatform());
			path += this->addToPath(this->type());
			path += this->addToPath(this->exec7z());
			return path;
		}

		std::string isStat(std::string chunk) {
			std::smatch match;

			this->stdOut.append(chunk);
			std::regex_search(chunk, match, this->percentReg);
			if (!match.empty())
				this->percent = std::stoi(*(--match.end()));
			return match.str();
		}

		SevenZipVector splitString() {
			std::regex re("[\r\n|\n|\r]");
			std::sregex_token_iterator
				first{this->stdOut.begin(), this->stdOut.end(), re, -1},
				last;
			SevenZipVector ret = { first, last };

			ret.erase(std::remove_if(ret.begin(), ret.end(), isEmpty), ret.end());
			std::for_each(ret.begin(), ret.end(), &trim);
			return ret;
		}

		SevenZipVector split(const std::string &str, const char &delimiter, bool trimStr = false) {
			SevenZipVector ret;
			std::string buff;

			for (auto n: str) {
				if (n != delimiter)
					buff.push_back(n);
				else if (n == delimiter && !buff.empty()) {
					buff.erase(std::remove_if(buff.begin(), buff.end(), isspace), buff.end());
					ret.push_back(buff);
					buff.clear();
				}
			}
			if (!buff.empty())
				ret.push_back(buff);
			if (trimStr)
				std::for_each(ret.begin(), ret.end(), &trim);
			return ret;
		}

		SevenZipList parseListOutput() {
			SevenZipVector lines = this->splitString();
			SevenZipList list = {
				{ "Path", "name" },
				{ "Size", "size" },
				{ "Packed Size", "compressed" },
				{ "Attributes", "attr" },
				{ "Modified", "dateTime" },
				{ "CRC", "crc" },
				{ "Method", "method" },
				{ "Block", "block" },
				{ "Encrypted", "encrypted" }
			},
			ret;

			for (SevenZipVector::iterator it = lines.begin(); it != lines.end(); it++) {
				SevenZipVector split = this->split(*it, ':', true);

				if (split.size() == 2) {
					auto check = list.find(split[0]);

					if (check != list.end()) {
						if (check->first == "dateTime") {
							auto splitTime = this->split(check->second, ' ', true);
							if (splitTime.size() == 2) {
								ret.insert({ "date", splitTime[0] });
								ret.insert({ "time", splitTime[1] });
							}
						} else
							ret.insert({ check->second, split[1] });
					}
				}
			}
			return ret;
		}
	public:
		SevenZipList cmd(SevenZipVector args, bool genStat = true, bool yesForAll = false) {
			this->percent = 0;
			this->stdOut.clear();
			this->stdOut = "";
		#ifndef _WIN32
			args.insert(args.begin(), "7za");
		#endif
			if (genStat)
				args.push_back("-bsp1");
			if (yesForAll)
				args.push_back("-y");

		#if _WIN32
			SECURITY_ATTRIBUTES saAttr;
			HANDLE hReadPipe, hWritePipe;
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			DWORD bytesRead;
			std::string command(this->path7z());
			char buffer[4096];

			saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle = TRUE;
			saAttr.lpSecurityDescriptor = NULL;
			
			for (SevenZipVector::const_iterator it = args.begin(); it != args.end(); it++)
				command += " " + *it;

			if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
     		throw new std::runtime_error(std::string("pipe failed: ") + strerror(errno));
			
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdOutput = hWritePipe;
			si.hStdError = hWritePipe;
			if (!CreateProcess(
				NULL, (LPSTR)command.c_str(), NULL,
				NULL, TRUE, CREATE_NEW_CONSOLE,
				NULL, NULL, &si, &pi)
			)
				throw new std::runtime_error(std::string("CreateProcess failed: ") + strerror(errno));
			CloseHandle(hWritePipe);

			while (ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
				if (bytesRead == 0)
					break;
				this->isStat(std::string(buffer));
			}
		#else
			pid_t pid;
			int stdPipe[2], bytes;
			char temp[256];

			auto execvArgs = new char* [args.size() + 1]();
			for (size_t i = 0; i < args.size(); i++) {
				execvArgs[i] = new char[(args[i].size() + 1)]();
				std::memcpy(execvArgs[i], args[i].c_str(), (args[i].size() + 1));
			}

			if (pipe(stdPipe) == -1)
				throw new std::runtime_error(std::string("pipe failed: ") + strerror(errno));
			if ((pid = fork()) == -1)
				throw new std::runtime_error(std::string("fork failed: ") + strerror(errno));
			if (pid == 0) {
				dup2(stdPipe[1], STDOUT_FILENO);
				close(stdPipe[0]); close(stdPipe[1]);
				execv(this->path7z().c_str(), execvArgs);

				std::string err = std::string("execv failed [");
				err.append(this->path7z());
				err.append("]: ");
				err.append(strerror(errno));
				throw new std::runtime_error(err);
			} else {
				close(stdPipe[1]);
				do {
					this->isStat(std::string(temp));
					std::memset(&(temp[0]), 0, 256);
				}
				while ((bytes = read(stdPipe[0], temp, sizeof(temp))) > 0);
			}

			for (size_t i = 0; execvArgs[i]; i++)
				delete [] execvArgs[i];
			delete [] execvArgs;
		#endif
			this->percent = 100;
			return this->parseListOutput();
		}

		SevenZipList pack(std::string pathToSrc, std::string pathToDest) {
			SevenZipVector args = { "a", pathToDest, pathToSrc };

			return this->cmd(args, true, true);
		}

		SevenZipList unpack(std::string pathToArchive, std::string pathToDest = "") {
			SevenZipVector args = { "x", pathToArchive };

			if (!pathToDest.empty())
				args.push_back("-o" + pathToDest);
			return this->cmd(args, true, true);
		}

		SevenZipList list(std::string pathToSrc) {
			SevenZipVector args = { "l", "-slt", "-ba", pathToSrc };

			return this->cmd(args, true, true);
		}
};

#endif
