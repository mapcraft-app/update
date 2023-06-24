#include "7zip.hpp"

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	try {
		SevenZip zip = SevenZip();
		SevenZipList s = zip.unpack("./1.20.zip", "./unpack");
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
