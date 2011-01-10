#include <string.h>

#include <tsxx/exceptions.hpp>

using tsxx::exceptions::stdio_error;

stdio_error::stdio_error(int number) throw()
	: number(number)
{
	description = ::strerror(number);
}

stdio_error::~stdio_error() throw()
{
}

const char *
stdio_error::what() const throw()
{
	return description.c_str();
}
