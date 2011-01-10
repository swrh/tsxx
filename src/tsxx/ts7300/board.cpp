#include <tsxx/ts7300.hpp>

using tsxx::ts7300::board;
using tsxx::ts7300::devices::xdio;
using tsxx::ts7300::devices::dio1;
using tsxx::ts7300::devices::lcd;
using tsxx::ts7300::devices::spi;

board::board(tsxx::system::memory &mem)
	: memory(mem), xdio2(memory, 1), dio1(memory), lcd(memory), spi(memory)
{
}

void
board::init()
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

xdio &
board::get_xdio2()
{
	return xdio2;
}

dio1 &
board::get_dio1()
{
	return dio1;
}

lcd &
board::get_lcd()
{
	return lcd;
}

spi &
board::get_spi()
{
	return spi;
}