#if !defined(_TSXX_PORTS_HPP_)
#define _TSXX_PORTS_HPP_

#include <tsxx/interfaces.hpp>
#include <tsxx/registers.hpp>
#include <tsxx/system.hpp>

namespace
tsxx
{

namespace
ports
{

template <class WordReg> class
wordport
{
public:
	wordport(tsxx::system::memory_region_window region)
		: mem(region), reg(mem.get_pointer())
	{
	}

	wordport(tsxx::system::memory_region_window region, unsigned int param)
		: mem(region), reg(mem.get_pointer(), param)
	{
	}

	// WordPort
public:
	typedef typename WordReg::word_type word_type;

	inline void
	write(word_type word)
	{
		reg.write(word);
	}

	inline word_type
	read()
	{
		return reg.read();
	}

protected:
	tsxx::system::memory_region_window mem;
	WordReg reg;

};

template <class WordPort> class
bitport
	: public tsxx::interfaces::binport
{
public:
	bitport(WordPort &port, unsigned int bitno)
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
	WordPort &wordport;
	const typename WordPort::word_type mask;

};

typedef wordport<tsxx::registers::reg8> port8;
typedef wordport<tsxx::registers::reg16> port16;
typedef wordport<tsxx::registers::reg32> port32;
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

	// WordPort
public:
	typedef typename WordPort::word_type word_type;

	void
	write(word_type word)
	{
		get_data_port().write(word);
	}

	word_type
	read()
	{
		return get_data_port().read();
	}

public:
	/**
	 * Sets the DIO pins direction.
	 *
	 * @param dir The direction of each pin: bits with 1 sets pin for
	 * output and with 0 for input.
	 */
	void
	set_dir(word_type dir)
	{
		ddr_port.write(dir);
	}

	word_type
	get_dir()
	{
		return ddr_port.read();
	}

public:
	inline WordPort &
	get_data_port()
	{
		return data_port;
	}

private:
	WordPort data_port;
	WordPort ddr_port;

};

}
}

#endif // !defined(_TSXX_PORTS_HPP_)
