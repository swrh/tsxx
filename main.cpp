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

#define LOG(x) (std::clog << __FILE__ << ":" << __LINE__ << ": " << (x) << std::endl)

#define FIXME() LOG("FIXME")
#define TODO() LOG("TODO")
#define XXX() LOG("XXX")

extern const char *__progname;

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

namespace tsxx {

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

	// Force data access using the "ldrb" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline uint8_t
	read()
	{
		volatile uint8_t ret;
		asm volatile (
			"ldrb %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	// Force data access using the "strb" instruction -- don't rely on
	// compiler optimization by using pointers.
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

	// Force data access using the "ldrh" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline uint16_t
	read()
	{
		volatile uint16_t ret;
		asm volatile (
			"ldrh %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	// Force data access using the "strh" instruction -- don't rely on
	// compiler optimization by using pointers.
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

	// Force data access using the "ldr" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline uint32_t
	read()
	{
		volatile uint32_t ret;
		asm volatile (
			"ldr %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	// Force data access using the "str" instruction -- don't rely on
	// compiler optimization by using pointers.
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
		: mem(region), reg(mem.get_pointer())
	{
	}

	wordport(CopyableMemoryRegion region, unsigned int param)
		: mem(region), reg(mem.get_pointer(), param)
	{
	}

	// Force data access using the "strb" instruction -- don't rely on
	// compiler optimization by using pointers.
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
	CopyableMemoryRegion mem;
	WordReg reg;

};

template <class WordPort> class
bitport
	: public tsxx::interfaces::binport
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

	// Force data access using the "strh" instruction -- don't rely on
	// compiler optimization by using pointers.
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

typedef wordport<tsxx::registers::reg8bits> port8;
typedef wordport<tsxx::registers::reg16bits> port16;
typedef wordport<tsxx::registers::reg32bits> port32;
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

	typename WordPort::word_type
	read()
	{
		return data_port.read();
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
	: public tsxx::ports::dioport<tsxx::ports::port8>
{
	// TODO not tested yet
private:
	enum { BASE_ADDR = 0x80840000 };
public:
	dio1(tsxx::memory &memory)
		: tsxx::ports::dioport<tsxx::ports::port8>(tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x04)), tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x14)))
	{
	}

};

/**
 * LCD port class.
 */
class
lcd
{
#define LCD_CMD_ROW0			(0x80 | 0x00)
#define LCD_CMD_ROW1			(0x80 | 0x40)
#define LCD_CMD_ROW2			(0x80 | 0x14)
#define LCD_CMD_ROW3			(0x80 | 0x54)
#define LCD_CMD_ROW(x)			(x == 1 ? LCD_CMD_ROW1 : x == 2 ? LCD_CMD_ROW2 : x == 3 ? LCD_CMD_ROW3 : LCD_CMD_ROW0)

#define LCD_CMD_CLEAR			0x0001
#define LCD_CMD_HOME			0x0002

#define LCD_CMD_ENTRY			0x0004
#define LCD_BIT_ENTRY_DIR_RIGHT		0x0002
#define LCD_BIT_ENTRY_DIR_LEFT		0x0000

#define LCD_CMD_CTRL			0x0008
#define LCD_BIT_CTRL_BLNK_OFF		0x0000
#define LCD_BIT_CTRL_BLNK_ON		0x0001
#define LCD_BIT_CTRL_CUR_OFF		0x0000
#define LCD_BIT_CTRL_CUR_ON		0x0002
#define LCD_BIT_CTRL_DSP_OFF		0x0000
#define LCD_BIT_CTRL_DSP_ON		0x0004

#define LCD_CMD_FNSET			0x0020
#define LCD_BIT_FNSET_FONT_5x8		0x0000
#define LCD_BIT_FNSET_FONT_5x11		0x0004
#define LCD_BIT_FNSET_NLINES_1LIN	0x0000
#define LCD_BIT_FNSET_NLINES_2LIN	0x0008
#define LCD_BIT_FNSET_DATLEN_4BIT	0x0000
#define LCD_BIT_FNSET_DATLEN_8BIT	0x0010

#define LCD_CMD_CGRAM			0x0040
#define LCD_CMD_DDRAM			0x0080
#define LCD_CMD_RDSTAT			0x0100
#define LCD_CMD_WRDATA			0x0200
#define LCD_CMD_RDDATA			0x0300
	// TODO not finished yet
private:
	enum { BASE_ADDR = 0x80840000 };
public:
	lcd(tsxx::memory &memory)
		: data(tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x00)), tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x10))),
		data7(tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x08)), tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x18))),
		data_mask(0x7f), data7_mask(0x01),
		data_bit_busy(0x80),
		ctrl(tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x40)), tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x44))),
		ctrl_bit_en(0x08), ctrl_bit_rs(0x10), ctrl_bit_wr(0x20)
	{
	}

	void
	init()
	{
		// Set LCD control pins as outputs.
		ctrl.set_dir(ctrl.get_dir() | ctrl_bit_en | ctrl_bit_rs | ctrl_bit_wr);

		// De-assert EN and RS.
		ctrl.write(ctrl.read() & ~(ctrl_bit_rs | ctrl_bit_en));

		usleep(15000);
		command(LCD_CMD_FNSET |
		    LCD_BIT_FNSET_FONT_5x8 |
		    LCD_BIT_FNSET_NLINES_2LIN |
		    LCD_BIT_FNSET_DATLEN_8BIT);
		usleep(4100);
		command(LCD_CMD_FNSET |
		    LCD_BIT_FNSET_FONT_5x8 |
		    LCD_BIT_FNSET_NLINES_2LIN |
		    LCD_BIT_FNSET_DATLEN_8BIT);
		usleep(100);
		command(LCD_CMD_FNSET |
		    LCD_BIT_FNSET_FONT_5x8 |
		    LCD_BIT_FNSET_NLINES_2LIN |
		    LCD_BIT_FNSET_DATLEN_8BIT);
		usleep(39);
		wait();

		command(LCD_CMD_FNSET |
		    LCD_BIT_FNSET_FONT_5x8 |
		    LCD_BIT_FNSET_NLINES_2LIN |
		    LCD_BIT_FNSET_DATLEN_8BIT);
		wait();

		command(LCD_CMD_ENTRY |
		    LCD_BIT_ENTRY_DIR_RIGHT);
		usleep(39);
		wait();

		command(LCD_CMD_CLEAR);
		usleep(1530);
		wait();

		command(LCD_CMD_CTRL |
		    LCD_BIT_CTRL_BLNK_OFF |
		    LCD_BIT_CTRL_CUR_OFF |
		    LCD_BIT_CTRL_DSP_ON);
		wait();

		command(LCD_CMD_HOME);
		usleep(1530);
		wait();
	}

	void
	print(std::string str)
	{
		print(str.c_str(), str.length());
	}

	void
	print(const void *p, std::size_t len)
	{
		// Set LCD data pins as outputs.
		data.set_dir(data.get_dir() | data_mask);
		data7.set_dir(data7.get_dir() | data7_mask);

		tsxx::ports::port8::word_type c = ctrl.read();

		for (const uint8_t *s = static_cast<const uint8_t *>(p); len-- > 0; s++) {
			// Write data to be sent.
			data.write((data.read() & ~data_mask) | (*s & data_mask));
			data7.write((data7.read() & ~data7_mask) | ((*s >> 7) & data7_mask));

			// Assert WR and RS.
			c = (c & ~ctrl_bit_wr) | ctrl_bit_rs;
			ctrl.write(c);

			// Sleep 100ns at least.
			nssleep(100);

			// Assert EN.
			c |= ctrl_bit_en;
			ctrl.write(c);

			// Sleep 300ns at least.
			nssleep(300);

			// De-assert EN.
			c &= ~ctrl_bit_en;
			ctrl.write(c);

			// Sleep 200ns at least.
			nssleep(200);
		}
	}

	void
	command(uint8_t cmd)
	{
		tsxx::ports::port8::word_type c = ctrl.read();

		// Set LCD data pins as outputs.
		data.set_dir(data.get_dir() | data_mask);
		data7.set_dir(data7.get_dir() | data7_mask);

		// 20110110 XXX Is this block really necessary?
#if 0
		// Assert RS.
		c |= ctrl_bit_rs;
		ctrl.write(c);
#endif

		// Write data to be sent.
		data.write((data.read() & ~data_mask) | (cmd & data_mask));
		data7.write((data7.read() & ~data7_mask) | ((cmd >> 7) & data7_mask));

		c &= ~(ctrl_bit_rs | ctrl_bit_wr); // de-assert RS, assert WR
		ctrl.write(c);

		// Sleep 100ns at least.
		nssleep(100);

		// step 3, assert EN
		c |= ctrl_bit_en;
		ctrl.write(c);

		// Sleep 300ns at least.
		nssleep(300);

		// step 5, de-assert EN
		c &= ~ctrl_bit_en; // de-assert EN
		ctrl.write(c);

		// Sleep 200ns at least.
		nssleep(200);
	}

	bool
	wait()
	{
		unsigned int tries;
		tsxx::ports::port8::word_type d, c = ctrl.read();

		// Set LCD data pins as inputs.
		data.set_dir(data.get_dir() & ~data_mask);
		data7.set_dir(data7.get_dir() & ~data7_mask);

		tries = 0;
		do {
			// De-assert RS and WR.
			c = (c | ctrl_bit_wr) & ~ctrl_bit_rs;
			ctrl.write(c);

			// Sleep 100ns at least.
			nssleep(100);

			// Assert EN.
			c |= ctrl_bit_en;
			ctrl.write(c);

			// Sleep 300ns at least.
			nssleep(300);

			// De-assert EN, read result

			d = (data.read() & data_mask) | ((data7.read() & data7_mask) << 7);

			// De-assert EN.
			c &= ~ctrl_bit_en;
			ctrl.write(c);

			// Sleep 200ns at least.
			nssleep(200);
		} while ((d & data_bit_busy) != 0 && tries++ < 1024);

		return (d & data_bit_busy) == 0;
	}

private:
	// Referenced in the manual as Port A (data) and C (data7).
	tsxx::ports::dioport<tsxx::ports::port8> data, data7;
	const tsxx::ports::port8::word_type data_mask, data7_mask;
	const tsxx::ports::port8::word_type data_bit_busy;

	// Referenced in the manual as Port H.
	tsxx::ports::dioport<tsxx::ports::port8> ctrl;
	const tsxx::ports::port8::word_type ctrl_bit_en, ctrl_bit_rs, ctrl_bit_wr;

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
	enum { BASE_ADDR = 0x72000040 };
public:
	/**
	 * @param n The number of the XDIO port. Can be 0 (XDIO1) or 1 (XDIO2).
	 */
	xdio(tsxx::memory &memory, unsigned int n)
		: conf(memory.get_region(BASE_ADDR + n * 4 + 0)),
		reg1(memory.get_region(BASE_ADDR + n * 4 + 1)),
		reg2(memory.get_region(BASE_ADDR + n * 4 + 2)),
		reg3(memory.get_region(BASE_ADDR + n * 4 + 3)),
		dio_port(reg1, reg2)
	{
		if (n > 1)
			throw tsxx::stdio_error(EINVAL); // FIXME use the correct exception

		mode = UNINITIALIZED;
	}

	void
	init()
	{
		read_conf();
	}

private:
	enum mode_cfg {
		MODE_DIO = 0x00,
		MODE_EDGEQUADCNTR = 0x01,
		MODE_INPULSTIMER = 0x02,
		MODE_PWM = 0x03,
		UNINITIALIZED,
	} mode;

	void
	read_conf()
	{
		tsxx::ports::port8::word_type c = conf.read();
		mode = static_cast<mode_cfg>((c >> 6) & 0x03);
	}

	void
	write_conf()
	{
		switch (mode) {
		case MODE_DIO:
		case MODE_EDGEQUADCNTR:
		case MODE_INPULSTIMER:
		case MODE_PWM:
			break;
		case UNINITIALIZED:
		default:
			throw tsxx::stdio_error(EINVAL); // FIXME use the correct exception
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
	tsxx::ports::dioport<tsxx::ports::port8> &
	get_dio()
	{
		if (mode != MODE_DIO)
			throw tsxx::stdio_error(EINVAL); // FIXME use the correct exception
		return dio_port;
	}

private:
	tsxx::ports::port8 conf, reg1, reg2, reg3;
	tsxx::ports::dioport<tsxx::ports::port8> dio_port;

};

class
spi
{
private:
	enum { BASE_ADDR = 0x808a0000 };
public:
	spi(tsxx::memory &memory)
		: ctrl(memory.get_region(BASE_ADDR + 0x04)),
		status(memory.get_region(BASE_ADDR + 0x0c)),
		data(memory.get_region(BASE_ADDR + 0x08)),
		tx_bit(ctrl, 4),
		busy_bit(status, 4),
		inp_bit(status, 2)
	{
	}

public:
	bool
	add_chip(unsigned int id, tsxx::interfaces::binport &cs)
	{
		if (chips.find(id) != chips.end())
			return false;

		cs.unset();

		chips.insert(std::pair<unsigned int, tsxx::interfaces::binport &>(id, cs));

		return true;
	}

	void
	init()
	{
		tx_bit.set();
		FIXME(); while (busy_bit.get());
		tx_bit.unset();
		FIXME(); while (busy_bit.get()); // Is this really necessary?
		FIXME(); while (inp_bit.get())
			(void)data.read();
	}

	bool
	writenread(unsigned int id, std::vector<uint8_t> &rw_data)
	{
		return writenread(id, rw_data, rw_data);
	}

	bool
	writenread(unsigned int id, const std::vector<uint8_t> &write_data, std::vector<uint8_t> &read_data)
	{
		std::map<unsigned int, tsxx::interfaces::binport &>::iterator chip = chips.find(id);
		if (chip != chips.end())
			return false;

		if (read_data.size() != write_data.size())
			read_data.resize(write_data.size());

		for (std::vector<uint8_t>::const_iterator w = write_data.begin(); w != write_data.end(); w++)
			data.write(*w);

		chip->second.set();

		tx_bit.set();
		FIXME(); while (busy_bit.get());

		chip->second.unset();

		for (std::vector<uint8_t>::iterator w = read_data.begin(); w != read_data.end(); w++)
			*w = data.read();
		tx_bit.unset();

		return true;
	}

protected:
	std::map<unsigned int, tsxx::interfaces::binport &> chips;

private:
	/// SPI registers.
	tsxx::ports::port16 ctrl, status, data;
	tsxx::ports::bport16 tx_bit, busy_bit, inp_bit;

};

}

class
board
{
public:
	board(tsxx::memory &mem)
		: memory(mem), xdio2(memory, 1), dio1(memory), lcd(memory), spi(memory)
	{
	}

	void
	init()
	{
		// ATTENTION: Always set this bit to 0 or else you might
		// "brick" your board forever while using the SPI bus. Setting
		// it to one will enable the EEPROM boot chip and you might end
		// up overwriting the boot magic word "CRUS".
		//
		// For reference check TS-7300 MANUAL of Apr 2010, page 14.
		//
		// PS: Believe me, I've done this. I've seen the RLOD (Red Led
		// Of Death). =(
		tsxx::ports::bport8 eeprom_cs_bit(memory.get_region(0x23000000), 0);
		eeprom_cs_bit.unset();

		xdio2.init();
		lcd.init();
		spi.init();
	}

	inline ts7300::devices::xdio &
	get_xdio2()
	{
		return xdio2;
	}

	inline ts7300::devices::dio1 &
	get_dio1()
	{
		return dio1;
	}

	inline ts7300::devices::lcd &
	get_lcd()
	{
		return lcd;
	}

	inline ts7300::devices::spi &
	get_spi()
	{
		return spi;
	}

private:
	tsxx::memory &memory;

	ts7300::devices::xdio xdio2;
	ts7300::devices::dio1 dio1;
	ts7300::devices::lcd lcd;
	ts7300::devices::spi spi;

};

}

int
main(int argc, char *argv[])
{
	try {
		tsxx::memory memory;
		if (!memory.open())
			err(EXIT_FAILURE, "error opening memory");

		ts7300::board ts(memory);
		ts.init();

		ts.get_lcd().print("testing...");
	} catch (tsxx::stdio_error &e) {
		std::cerr << __progname << ": stdio exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	} catch (tsxx::exception &e) {
		std::cerr << __progname << ": tsxx exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	} catch (std::exception &e) {
		std::cerr << __progname << ": standard exception: " << e.what() << std::endl;
		exit(EXIT_FAILURE);

	}

	std::cout << "done." << std::endl;
	exit(EXIT_SUCCESS);
}
