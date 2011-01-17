#if !defined(_TSXX_SYSTEM_HPP_)
#define _TSXX_SYSTEM_HPP_

#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

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
	file_descriptor(int fd = -1);
	~file_descriptor();

	int get_value() const;
	void close();
	bool is_valid();

private:
	int fd;

};
typedef boost::shared_ptr<file_descriptor> file_descriptor_ptr;

class
memory_region
: private boost::noncopyable
{
public:
	memory_region(file_descriptor_ptr fd);
	~memory_region();

	bool map(std::size_t len, off_t offset);
	void unmap();

	void *get_pointer();

private:
	file_descriptor_ptr fd;
	void *pointer;
	std::size_t length;

};
typedef boost::shared_ptr<memory_region> memory_region_ptr;

class
memory_region_window
{
public:
	memory_region_window(memory_region_ptr reg, off_t off);

	void *get_pointer();

private:
	memory_region_ptr region;
	off_t offset;

};

class
memory
: private boost::noncopyable // XXX check if this is really needed
{
public:
	memory();

	std::size_t get_region_size() const;

	bool is_opened();
	bool open();
	void try_close();

	memory_region_window get_region(off_t address);

private:
	file_descriptor_ptr fd;
	std::map<off_t, memory_region_ptr> memory_regions;
	std::size_t region_size;

};

// TODO Calibrate this function.
/**
 * @warning This function hasn't been calibrated yet.
 */
inline void
nssleep(unsigned int ns)
{
	volatile unsigned int loop = ns * 5;
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
