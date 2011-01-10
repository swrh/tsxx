#include <errno.h>
#include <sys/mman.h>

#include <tsxx/system.hpp>

using tsxx::system::memory_region;

memory_region::memory_region(file_descriptor_ptr fd)
{
	if (fd.get() == NULL || !fd->is_valid())
		throw tsxx::exceptions::stdio_error(EINVAL);

	this->fd = fd;
	pointer = MAP_FAILED;
	length = 0;
}

memory_region::~memory_region()
{
	unmap();
}

bool
memory_region::map(std::size_t len, off_t offset)
{
	if (pointer != MAP_FAILED) {
		errno = EBADF;
		return false;
	}

	pointer = ::mmap(0, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd->get_value(), offset);
	if (pointer == MAP_FAILED)
		return false;

	length = len;

	return true;
}

void
memory_region::unmap()
{
	if (pointer == MAP_FAILED)
		return;

	(void)::munmap(pointer, length);

	pointer = MAP_FAILED;
	length = 0;
}

void *
memory_region::get_pointer()
{
	if (pointer == MAP_FAILED)
		return NULL;
	return pointer;
}
