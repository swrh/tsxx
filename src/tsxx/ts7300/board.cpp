#include <tsxx/ts7300.hpp>

using tsxx::ts7300::board;
using tsxx::ts7300::devices::xdio;
using tsxx::ts7300::devices::dio1;
using tsxx::ts7300::devices::lcd;
using tsxx::ts7300::devices::spi;

board::board(tsxx::system::memory &mem)
	: memory(mem), xdio1(memory, 0), xdio2(memory, 1), dio1(memory), lcd(memory), spi(memory)
{
}

void
board::init()
{
	// Unlock software lock.
	{
		tsxx::ports::port32 port(memory.get_region(0x809300c0));
		port.write(0x000000aa);
	}

	// Disable DMA/enable GPIO pins.
	{
		tsxx::ports::port32 port(memory.get_region(0x80930080));
		port.write(0x08140d00);
	}

	// ATTENTION: Always set this bit to 0 or else you might
	// "brick" your board forever while using the SPI bus. Setting
	// it to one will enable the EEPROM boot chip and you might end
	// up overwriting the boot magic word "CRUS".
	//
	// For reference check TS-7300 MANUAL of Apr 2010, page 14.
	//
	// PS: Believe me, I've done this. I've seen the RLOD (Red Led
	// Of Death). =(
	{
		tsxx::ports::port8 eeprom_cs_port(memory.get_region(0x23000000));
		tsxx::ports::bport8 eeprom_cs_bit(eeprom_cs_port, 0);
		eeprom_cs_bit.unset();
	}

	xdio1.init();
	xdio2.init();

	// If we initialize the lcd object it will send commands to LCD device.
	// As we don't know the TS-7300 board is connected to an LCD device, we
	// don't do that.
	//lcd.init();

	spi.init();
}

xdio &
board::get_xdio1()
{
	return xdio1;
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
