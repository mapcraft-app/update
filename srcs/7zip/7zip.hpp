#ifndef SEVENZIP_HPP
#define SEVENZIP_HPP

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define PLATFORM TOSTRING(TARGET)

#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "trim.hpp"

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
			if (std::strcmp(PLATFORM, "windows") == 0)
				return "win";
			if (std::strcmp(PLATFORM, "macos") == 0)
				return "mac";
			return "unix";
		};

		std::string exec7z() {
			if (std::strcmp(PLATFORM, "windows") == 0)
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

		std::filesystem::path path7z() {
			std::filesystem::path path = std::filesystem::current_path();
			path /= "srcs";
			path /= "7zip";
			path /= this->convertPlatform();
			path /= this->type();
			path /= this->exec7z();
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
			pid_t pid;
			int stdPipe[2], status, bytes;
			char temp[256];

			this->percent = 0;
			this->stdOut.clear();
			this->stdOut = "";
			args.insert(args.begin(), "7za");
			if (genStat)
				args.push_back("-bsp1");
			if (yesForAll)
				args.push_back("-y");

			auto execvArgs = new char* [args.size() + 1]();
			for (size_t i = 0; i < args.size(); i++) {
				execvArgs[i] = new char[(args[i].size() + 1)]();
				std::memcpy(execvArgs[i], args[i].c_str(), (args[i].size() + 1));
			}
			execvArgs[args.size()] = NULL;

			if (pipe(stdPipe) == -1)
				throw new std::exception();
			if ((pid = fork()) == -1)
				throw new std::exception();
			if (pid == 0) {
				dup2(stdPipe[1], STDOUT_FILENO);
				close(stdPipe[0]); close(stdPipe[1]);
				execv(this->path7z().c_str(), execvArgs);
				throw new std::exception();
			} else {
				close(stdPipe[1]);
				do {
					this->isStat(std::string(temp));
					std::memset(&(temp[0]), 0, 256);
				}
				while ((bytes = read(stdPipe[0], temp, sizeof(temp))) > 0);
				waitpid(pid, &status, 0);
			}

			this->percent = 100;
			for (size_t i = 0; execvArgs[i]; i++)
				delete [] execvArgs[i];
			delete [] execvArgs;
			return this->parseListOutput();
		}

		SevenZipList pack(std::string pathToSrc, std::string pathToDest) {
			SevenZipVector args = { "a", pathToDest, pathToSrc };

			return this->cmd(args, true, true);
		}

		SevenZipList unpack(std::string pathToArchive, std::string pathToDest = "") {
			SevenZipVector args = { "x", pathToArchive };

			if (pathToDest.empty())
				return this->cmd(args, true, true);
			args.push_back("-o" + pathToDest);
			return this->cmd(args, true, true);
		}

		SevenZipList list(std::string pathToSrc) {
			SevenZipVector args = { "l", "-slt", "-ba", pathToSrc };

			return this->cmd(args, true, true);
		}
};

#endif
