#include <iostream>
#include "7zip.hpp"
#include "spawn.hpp"
#include "filesystem.hpp"

/**
 * argv[1]: archive path
 * argv[2]: dir unpack
 * argv[3]: executable path
 * argv[4]: delete archive after unpack {optional}
*/
int main(int argc, char **argv) {
	SevenZip zip = SevenZip();
	bool deleteAfter = false;

	std::cout << current_path() << std::endl;

	if (argc < 4 || argc > 6) {
		std::cerr << "Arguments:\n-1 {string} archive path\n-2 {string} unpack dir\n-3 {string} executable path\n-4 {boolean | optional} delete archive after unpack" << std::endl;
		return 1;
	}
	if (argc == 5 && argv[4] && std::strcmp(argv[4], "true") == 0)
		deleteAfter = true;
	try {
		std::cout << "Unpack " << argv[1] << " to " << argv[2] << std::endl;
		SevenZipList s = zip.unpack(argv[1], argv[2]);
		if (deleteAfter) {
			std::cout << "Delete archive" << std::endl;
			rm(argv[1]);
		}
		std::cout << "Fork and exec software" << std::endl;
		spawnProgram(argv[3]);
	} catch (std::exception &e) {
		std::cerr << "ERROR: " <<  e.what() << std::endl;
	}
	return 0;
}
