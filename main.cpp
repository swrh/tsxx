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

#define SPI_PAGE			0x808a0000
#define XDIO_PAGE			0x72000040

#define LOG(x) (std::clog << __FILE__ << ":" << __LINE__ << ": " << (x) << std::endl)

#define FIXME() LOG("FIXME")
#define TODO() LOG("TODO")
#define XXX() LOG("XXX")

extern const char *__progname;

namespace ts7x {

class
exception
: public std::exception
{

};

class
stdio_error
: public exception
{
public:
	stdio_error(int number) throw()
		: number(number)
	{
		description = ::strerror(number);
	}

	~stdio_error() throw()
	{
	}

	virtual const char *
	what() const throw()
	{
		return description.c_str();
	}

private:
	const int number;
	std::string description;

};

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
			throw stdio_error(EINVAL);

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
			throw stdio_error(EINVAL);

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
			throw stdio_error(errno);
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

		int val = ::open("/dev/mem", O_RDWR);
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
			throw stdio_error(EBADF);

		off_t offset = address % get_region_size();
		address -= offset;

		std::map<off_t, memory_region_ptr>::iterator it = memory_regions.find(address);
		if (it != memory_regions.end())
			return memory_region_window(it->second, offset);

		memory_region_ptr region(new memory_region(fd));
		if (!region->map(get_region_size(), address))
			throw stdio_error(errno);

		memory_regions.insert(std::pair<off_t, memory_region_ptr>(address, region));

		return memory_region_window(region, offset);
	}

private:
	file_descriptor_ptr fd;
	std::map<off_t, memory_region_ptr> memory_regions;
	std::size_t region_size;

};

namespace registers {

// WordReg
class
reg8bits
{
public:
	typedef uint8_t word_type;

	reg8bits(void *p)
		: address(reinterpret_cast<unsigned long>(p))
	{
		if (p == NULL)
			throw stdio_error(EINVAL);
	}

	inline uint8_t
	read()
	{
		uint8_t ret;
		asm volatile (
			"ldrb %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	inline void
	write(uint8_t dat)
	{
		asm volatile (
			"strb %1, [ %0 ]\n"
			:
			: "r" (address), "r" (dat)
			: "memory"
		);
	}

private:
	const unsigned long address;

};

// WordReg
class
reg16bits
{
public:
	typedef uint16_t word_type;

	reg16bits(void *p)
		: address(reinterpret_cast<unsigned long>(p))
	{
		if (p == NULL)
			throw stdio_error(EINVAL);
	}

	// We can't expect that a dereference of an unsigned short * always
	// produces a ldrh or strh since the compiler may choose to use
	// a byte write instead. Hence, we emit the peeks and pokes using
	// inline assembler. --JO
	inline uint16_t
	read()
	{
		uint16_t ret;
		asm volatile (
			"ldrh %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	inline void
	write(uint16_t dat)
	{
		asm volatile (
			"strh %1, [ %0 ]\n"
			:
			: "r" (address), "r" (dat)
			: "memory"
		);
	}

private:
	const unsigned long address;

};

// WordReg
class
reg32bits
{
public:
	typedef uint32_t word_type;

	reg32bits(void *p)
		: address(reinterpret_cast<unsigned long>(p))
	{
		if (p == NULL)
			throw stdio_error(EINVAL);
	}

	inline uint32_t
	read()
	{
		uint32_t ret;
		asm volatile (
			"ldr %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	inline void
	write(uint32_t dat)
	{
		asm volatile (
			"str %1, [ %0 ]\n"
			:
			: "r" (address), "r" (dat)
			: "memory"
		);
	}

private:
	const unsigned long address;

};

}

namespace interfaces {

class
binport
{
public:
	virtual void set() = 0;
	virtual void unset() = 0;

};

}

namespace ports {

// WordPort
template <class WordReg, class CopyableMemoryRegion = memory_region_window> class
wordport
{
public:
	wordport(CopyableMemoryRegion region)
		: reg(region.get_pointer())
	{
	}

	wordport(CopyableMemoryRegion region, unsigned int param)
		: reg(region.get_pointer(), param)
	{
	}

	inline void
	write(typename WordReg::word_type word)
	{
		reg.write(word);
	}

	inline typename WordReg::word_type
	read()
	{
		return reg.read();
	}

	typedef typename WordReg::word_type word_type;

protected:
	WordReg reg;

};

template <class WordPort> class
bitport
	: public ts7x::interfaces::binport
{
public:
	bitport(WordPort port, unsigned int bitno)
		: wordport(port), mask(1 << bitno)
	{
	}

	inline void
	unset()
	{
		write(false);
	}

	inline void
	set()
	{
		write(true);
	}

	inline bool
	get()
	{
		return read();
	}

protected:
	void
	write(bool state)
	{
		typename WordPort::word_type dat = wordport.read();

		if (state)
			dat |= mask;
		else
			dat &= ~mask;

		wordport.write(dat);
	}

	bool
	read()
	{
		return (wordport.read() & mask) == mask;
	}

private:
	WordPort wordport;
	const typename WordPort::word_type mask;

};

typedef wordport<ts7x::registers::reg8bits> port8;
typedef wordport<ts7x::registers::reg16bits> port16;
typedef wordport<ts7x::registers::reg32bits> port32;
typedef bitport<port8> bport8;
typedef bitport<port16> bport16;
typedef bitport<port32> bport32;

/**
 * DIO (GPIO) port class.
 */
template <class WordPort> class
dioport
{
public:
	dioport(WordPort data, WordPort dir)
		: data_port(data), ddr_port(dir)
	{
	}

	/**
	 * Sets the DIO pins direction.
	 *
	 * @param dir The direction of each pin: bits with 1 sets pin for
	 * output and with 0 for input.
	 */
	void
	set_dir(typename WordPort::word_type dir)
	{
		ddr_port.write(dir);
	}

	typename WordPort::word_type
	get_dir()
	{
		return ddr_port.read();
	}

	void
	write(typename WordPort::word_type word)
	{
		data_port.write(word);
	}

private:
	WordPort data_port;
	WordPort ddr_port;

};

}

}

namespace ts7300 {

namespace devices {

/**
 * DIO1 port class.
 */
class
dio1
	: public ts7x::ports::dioport<ts7x::ports::port8>
{
	// TODO not yet tested
private:
	enum { BASE_ADDR = 0x80840000 };
public:
	dio1(ts7x::memory &memory)
		: ts7x::ports::dioport<ts7x::ports::port8>(ts7x::ports::port8(memory.get_region(BASE_ADDR + 0x04)), ts7x::ports::port8(memory.get_region(BASE_ADDR + 0x14)))
	{
	}

};

/**
 * LCD port class.
 */
class
lcd
{
	// TODO not yet finished
private:
	enum { BASE_ADDR = 0x80840000 };
public:
	lcd(ts7x::memory &memory)
		: lcd_data(ts7x::ports::port8(memory.get_region(BASE_ADDR + 0x00)), ts7x::ports::port8(memory.get_region(BASE_ADDR + 0x10))),
		lcd_data7(ts7x::ports::port8(memory.get_region(BASE_ADDR + 0x08)), ts7x::ports::port8(memory.get_region(BASE_ADDR + 0x18))),
		mask_lcd_data(0x7f),
		mask_lcd_data7(0x01)
	{
	}

	void
	init()
	{
		lcd_data.set_dir(mask_lcd_data | lcd_data.get_dir());
		lcd_data7.set_dir(mask_lcd_data7 | lcd_data.get_dir());
	}

private:
	ts7x::ports::dioport<ts7x::ports::port8> lcd_data;
	ts7x::ports::dioport<ts7x::ports::port8> lcd_data7;

	const ts7x::ports::port8::word_type mask_lcd_data, mask_lcd_data7;

};

/**
 * XDIO port class.
 *
 * The only implemented mode (yet) is DIO (GPIO) mode (set_mode_dio()).
 */
class
xdio
{
private:
	enum { BASE_ADDR = XDIO_PAGE };
public:
	/**
	 * @param n The number of the XDIO port. Can be 0 (XDIO1) or 1 (XDIO2).
	 */
	xdio(ts7x::memory &memory, unsigned int n)
		: conf(memory.get_region(BASE_ADDR + n * 4 + 0)),
		reg1(memory.get_region(BASE_ADDR + n * 4 + 1)),
		reg2(memory.get_region(BASE_ADDR + n * 4 + 2)),
		reg3(memory.get_region(BASE_ADDR + n * 4 + 3)),
		dio_port(reg1, reg2)
	{
		if (n > 1)
			throw ts7x::stdio_error(EINVAL); // FIXME use the correct exception
		mode = INVALID;
	}

private:
	enum mode_cfg {
		MODE_DIO = 0x00,
		MODE_EDGEQUADCNTR = 0x01,
		MODE_INPULSTIMER = 0x02,
		MODE_PWM = 0x03,
		INVALID,
	} mode;

	void
	write_conf()
	{
		switch (mode) {
		case MODE_DIO:
		case MODE_EDGEQUADCNTR:
		case MODE_INPULSTIMER:
		case MODE_PWM:
			break;
		case INVALID:
		default:
			throw ts7x::stdio_error(EINVAL); // FIXME use the correct exception
		}

		conf.write(mode << 6);
	}

	// Operation mode setting methods.
public:
	void
	set_mode_dio()
	{
		mode = MODE_DIO;
		write_conf();
	}

	// DIO methods and variables.
public:
	/**
	 * Returns the DIO port.
	 */
	ts7x::ports::dioport<ts7x::ports::port8> &
	get_dio()
	{
		if (mode != MODE_DIO)
			throw ts7x::stdio_error(EINVAL); // FIXME use the correct exception
		return dio_port;
	}

private:
	ts7x::ports::port8 conf, reg1, reg2, reg3;
	ts7x::ports::dioport<ts7x::ports::port8> dio_port;

};

class
spi
{
private:
	enum { BASE_ADDR = SPI_PAGE };
public:
	spi(ts7x::memory &memory, unsigned long addr)
		: control_port(memory.get_region(BASE_ADDR + 0x04)),
		status_port(memory.get_region(BASE_ADDR + 0x0c)),
		data_port(memory.get_region(BASE_ADDR + 0x08)),
		tx_bit(control_port, 4),
		busy_bit(status_port, 4),
		inp_bit(status_port, 2)
	{
	}

	~spi()
	{
		clear();
	}

public:
	bool
	add_chip(unsigned int id, ts7x::interfaces::binport *cs)
	{
		if (cs == NULL || chips.find(id) != chips.end())
			return false;

		cs->unset();

		chips.insert(std::pair<unsigned int, ts7x::interfaces::binport *>(id, cs));

		return true;
	}

	void
	clear()
	{
		std::map<unsigned int, ts7x::interfaces::binport *>::iterator cs;
		for (cs = chips.begin(); cs != chips.end(); cs++)
			delete cs->second;
		chips.clear();
	}

	void
	init()
	{
		tx_bit.set();
		FIXME(); while (busy_bit.get());
		tx_bit.unset();
		FIXME(); while (busy_bit.get());
		FIXME(); while (inp_bit.get())
			(void)data_port.read();
	}

	bool
	writenread(unsigned int id, std::vector<uint8_t> rw_data)
	{
		return writenread(id, rw_data, rw_data);
	}

	bool
	writenread(unsigned int id, const std::vector<uint8_t> write_data, std::vector<uint8_t> read_data)
	{
		std::map<unsigned int, ts7x::interfaces::binport *>::iterator chip = chips.find(id);
		if (chip != chips.end())
			return false;

		for (std::vector<uint8_t>::const_iterator w = write_data.begin(); w != write_data.end(); w++)
			data_port.write(*w);

		chip->second->set();

		tx_bit.set();
		FIXME(); while (busy_bit.get());

		chip->second->unset();

		for (std::vector<uint8_t>::iterator w = read_data.begin(); w != read_data.end(); w++)
			*w = data_port.read();
		tx_bit.unset();

		return true;
	}

protected:
	std::map<unsigned int, ts7x::interfaces::binport *> chips;

private:
	/// SPI control register
	ts7x::ports::port16 control_port;
	/// SPI status register
	ts7x::ports::port16 status_port;
	/// SPI data register
	ts7x::ports::port16 data_port;

	ts7x::ports::bport16 tx_bit, busy_bit, inp_bit;

};

}

class
board
{
public:
	board(ts7x::memory &memory)
		: xdio2(memory, 1), dio1(memory), lcd(memory)
	{
	}

	ts7300::devices::xdio &
	get_xdio2()
	{
		return xdio2;
	}

	ts7300::devices::dio1 &
	get_dio1()
	{
		return dio1;
	}

	ts7300::devices::lcd &
	get_lcd()
	{
		return lcd;
	}

private:
	ts7300::devices::xdio xdio2;
	ts7300::devices::dio1 dio1;
	ts7300::devices::lcd lcd;

};

}

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
		ts7x::memory memory;
		std::cout << "opening memory..." << std::endl;
		if (!memory.open())
			err(EXIT_FAILURE, "error opening memory");

		std::cout << "allocating registers..." << std::endl;
		ts7x::ports::port8 data(memory.get_region(0x80840004));
		ts7x::ports::port8 ddr(memory.get_region(0x80840014));

		std::cout << "dumping registers..." << std::endl;
		std::cout << "\tddr: " << num2hex(ddr.read()) << std::endl;
		std::cout << "\tdata: " << num2hex(data.read()) << std::endl;

		ts7300::board ts(memory);

		std::cout << "dumping registers..." << std::endl;
		std::cout << "\tddr: " << num2hex(ddr.read()) << std::endl;
		std::cout << "\tdata: " << num2hex(data.read()) << std::endl;

		std::cout << "setting registers..." << std::endl;
		ts.get_xdio2().set_mode_dio();

		std::cout << "dumping registers..." << std::endl;
		std::cout << "\tddr: " << num2hex(ddr.read()) << std::endl;
		std::cout << "\tdata: " << num2hex(data.read()) << std::endl;

		std::cout << "setting registers..." << std::endl;
		ts.get_dio1().set_dir(0x01);

		std::cout << "dumping registers..." << std::endl;
		std::cout << "\tddr: " << num2hex(ddr.read()) << std::endl;
		std::cout << "\tdata: " << num2hex(data.read()) << std::endl;

		std::cout << "setting registers..." << std::endl;
		ts.get_dio1().write(0x01);

		std::cout << "dumping registers..." << std::endl;
		std::cout << "\tddr: " << num2hex(ddr.read()) << std::endl;
		std::cout << "\tdata: " << num2hex(data.read()) << std::endl;

		std::cout << "setting registers..." << std::endl;
		ddr.write(0xff);
		data.write(0x12);

		std::cout << "dumping registers..." << std::endl;
		std::cout << "\tddr: " << num2hex(ddr.read()) << std::endl;
		std::cout << "\tdata: " << num2hex(data.read()) << std::endl;

	} catch (ts7x::stdio_error &e) {
		std::cerr << __progname << ": stdio exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	} catch (ts7x::exception &e) {
		std::cerr << __progname << ": ts7x exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	} catch (std::exception &e) {
		std::cerr << __progname << ": standard exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	}

	std::cout << "done." << std::endl;
	exit(EXIT_SUCCESS);
}
