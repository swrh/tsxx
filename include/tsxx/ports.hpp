#if !defined(_TSXX_PORTS_HPP_)
#define _TSXX_PORTS_HPP_

#include <tsxx/registers.hpp>
#include <tsxx/system.hpp>

namespace tsxx
{
namespace ports
{

// WordPort
template <class WordReg, class CopyableMemoryRegion = tsxx::system::memory_region_window> class
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

#endif // !defined(_TSXX_PORTS_HPP_)
