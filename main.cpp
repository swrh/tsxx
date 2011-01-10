/**
 * TS-7x00 Linux library.
 *
 * All magic numbers used here have been extracted from the TS-7300 manual of
 * Apr, 2010.
 */

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <tsxx.hpp>

extern const char *__progname;

template <typename T> std::string
num2hex(T value)
{
	std::ostringstream stream;

	stream << "0x"
		<< std::setfill('0')
		<< std::setw(sizeof(T) * 2)
		<< std::setbase(16)
		<< static_cast<unsigned long long int>(value);

	return stream.str();
}

int
main(int argc, char *argv[])
{
	try {
		tsxx::system::memory memory;
		if (!memory.open())
			err(EXIT_FAILURE, "error opening memory");

		tsxx::ts7300::board ts(memory);
		ts.init();

		ts.get_lcd().print("testing...");
	} catch (tsxx::exceptions::stdio_error &e) {
		std::cerr << __progname << ": stdio exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	} catch (tsxx::exceptions::exception &e) {
		std::cerr << __progname << ": tsxx exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	} catch (std::exception &e) {
		std::cerr << __progname << ": standard exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	}

	std::cout << "done." << std::endl;
	exit(EXIT_SUCCESS);
}
