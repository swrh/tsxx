#include <errno.h>

#include <tsxx/registers.hpp>

#include <tsxx/exceptions.hpp>

using tsxx::registers::reg32;

reg32::reg32(void *p)
	: address(reinterpret_cast<unsigned long>(p))
{
	if (p == NULL)
		throw tsxx::exceptions::stdio_error(EINVAL);
}