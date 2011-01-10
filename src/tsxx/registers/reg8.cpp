#include <errno.h>

#include <tsxx/registers.hpp>

#include <tsxx/exceptions.hpp>

using tsxx::registers::reg8;

reg8::reg8(void *p)
	: address(reinterpret_cast<unsigned long>(p))
{
	if (p == NULL)
		throw tsxx::exceptions::stdio_error(EINVAL);
}
