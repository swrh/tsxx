#if !defined(_TSXX_SYSTEM_HPP_)
#define _TSXX_SYSTEM_HPP_

#include <tsxx/exceptions.hpp>

#define LOG(x) (std::clog << __FILE__ << ":" << __LINE__ << ": " << (x) << std::endl)
#define FIXME() LOG("FIXME")
#define TODO() LOG("TODO")
#define XXX() LOG("XXX")

namespace tsxx
{
namespace system
{

class
file_descriptor
: private boost::noncopyable
{
public:
	file_descriptor(int fd = -1)
	{
		this->fd = fd;
	}

	~file_descriptor()
	{
		close();
	}

	int
	get_value() const
	{
		return fd;
	}

	void
	close()
	{
		if (!is_valid())
			return;

		::close(fd);
		fd = -1;
	}

	bool
	is_valid()
	{
		return get_value() >= 0;
	}

private:
	int fd;

};
typedef boost::shared_ptr<file_descriptor> file_descriptor_ptr;

class
memory_region
: private boost::noncopyable
{
public:
	memory_region(file_descriptor_ptr fd)
	{
		if (fd.get() == NULL || !fd->is_valid())
			throw tsxx::exceptions::stdio_error(EINVAL);

		this->fd = fd;
		pointer = MAP_FAILED;
		length = 0;
	}

	~memory_region()
	{
		unmap();
	}

	bool
	map(std::size_t len, off_t offset)
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
	unmap()
	{
		if (pointer == MAP_FAILED)
			return;

		(void)::munmap(pointer, length);

		pointer = MAP_FAILED;
		length = 0;
	}

	void *
	get_pointer()
	{
		if (pointer == MAP_FAILED)
			return NULL;
		return pointer;
	}

private:
	file_descriptor_ptr fd;
	void *pointer;
	std::size_t length;

};
typedef boost::shared_ptr<memory_region> memory_region_ptr;

// CopyableMemoryRegion
class
memory_region_window
{
public:
	memory_region_window(memory_region_ptr reg, off_t off)
	{
		if (reg.get() == NULL)
			throw tsxx::exceptions::stdio_error(EINVAL);

		region = reg;
		offset = off;
	}

	void *
	get_pointer()
	{
		void *p = region->get_pointer();
		if (p == NULL)
			return NULL;

		return reinterpret_cast<uint8_t *>(p) + offset;
	}

private:
	memory_region_ptr region;
	off_t offset;

};

class
memory
: private boost::noncopyable // XXX check if this is really needed
{
public:
	memory()
	{
		if (getpagesize() < 0)
			throw tsxx::exceptions::stdio_error(errno);
		region_size = static_cast<std::size_t>(getpagesize());
	}

	std::size_t
	get_region_size() const
	{
		return region_size;
	}

	bool
	open()
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
	try_close()
	{
		if (!is_opened())
			return;

		memory_regions.clear();
	}

	bool
	is_opened()
	{
		if (fd.get() == NULL)
			return false;
		return fd->is_valid();
	}

	memory_region_window
	get_region(off_t address)
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

private:
	file_descriptor_ptr fd;
	std::map<off_t, memory_region_ptr> memory_regions;
	std::size_t region_size;

};

/**
 * @warning This function hasn't been calibrated yet.
 */
inline void
nssleep(unsigned int ns)
{
	volatile unsigned int loop = ns * 3;
	asm volatile (
		"1:\n"
		"subs %1, %1, #1;\n"
		"bne 1b;\n"
		: "=r" ((loop)) : "r" ((loop))
	);
}

}
}

#endif // !defined(_TSXX_SYSTEM_HPP_)
