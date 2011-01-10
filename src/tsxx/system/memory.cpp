#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <tsxx/system.hpp>

using tsxx::system::memory;
using tsxx::system::memory_region_window;

memory::memory()
{
	if (getpagesize() < 0)
		throw tsxx::exceptions::stdio_error(errno);
	region_size = static_cast<std::size_t>(getpagesize());
}

std::size_t
memory::get_region_size() const
{
	return region_size;
}

bool
memory::open()
{
	if (is_opened()) {
		errno = EINVAL;
		return false;
	}

	// We must use O_SYNC argument or else we WILL crash or get
	// random access while acessing registers.
	int val = ::open("/dev/mem", O_RDWR | O_SYNC);
	if (val == -1)
		return false;

	fd.reset(new file_descriptor(val));

	return true;
}

void
memory::try_close()
{
	if (!is_opened())
		return;

	memory_regions.clear();
}

bool
memory::is_opened()
{
	if (fd.get() == NULL)
		return false;
	return fd->is_valid();
}

memory_region_window
memory::get_region(off_t address)
{
	if (!is_opened())
		throw tsxx::exceptions::stdio_error(EBADF);

	off_t offset = address % get_region_size();
	address -= offset;

	std::map<off_t, memory_region_ptr>::iterator it = memory_regions.find(address);
	if (it != memory_regions.end())
		return memory_region_window(it->second, offset);

	memory_region_ptr region(new memory_region(fd));
	if (!region->map(get_region_size(), address))
		throw tsxx::exceptions::stdio_error(errno);

	memory_regions.insert(std::pair<off_t, memory_region_ptr>(address, region));

	return memory_region_window(region, offset);
}
