#if !defined(_TSXX_TS7300_HPP_)
#define _TSXX_TS7300_HPP_

#include <tsxx/ts7300/devices.hpp>

namespace tsxx
{
namespace ts7300
{

class
board
{
public:
	board(tsxx::system::memory &mem)
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
	tsxx::system::memory &memory;

	ts7300::devices::xdio xdio2;
	ts7300::devices::dio1 dio1;
	ts7300::devices::lcd lcd;
	ts7300::devices::spi spi;

};

}
}

#endif // !defined(_TSXX_TS7300_HPP_)
