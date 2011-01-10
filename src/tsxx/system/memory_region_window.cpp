#include <errno.h>

#include <tsxx/system.hpp>

using tsxx::system::memory_region_window;

memory_region_window::memory_region_window(memory_region_ptr reg, off_t off)
{
	if (reg.get() == NULL)
		throw tsxx::exceptions::stdio_error(EINVAL);

	region = reg;
	offset = off;
}

void *
memory_region_window::get_pointer()
{
	void *p = region->get_pointer();
	if (p == NULL)
		return NULL;

	return reinterpret_cast<uint8_t *>(p) + offset;
}
