#include <limits.h>

#include <iostream>

#include <tsxx/ts7300/devices.hpp>

using tsxx::ts7300::devices::spi;

spi::spi(tsxx::system::memory &memory)
	: ctrl(memory.get_region(BASE_ADDR + 0x04)),
	status(memory.get_region(BASE_ADDR + 0x0c)),
	data(memory.get_region(BASE_ADDR + 0x08)),
	tx_bit(ctrl, 4),
	busy_bit(status, 4),
	inp_bit(status, 2)
{
}

void
spi::init()
{
	tx_bit.set();
	FIXME(); while (busy_bit.get());
	tx_bit.unset();
	FIXME(); while (busy_bit.get()); // Is this really necessary?
	FIXME(); while (inp_bit.get())
		(void)data.read();
}

void
spi::write_read(tsxx::interfaces::binport &cs, const void *wrp, std::size_t wrsiz, void *rdp, std::size_t rdsiz)
{
	const uint8_t *cp;
	uint8_t *p;

	if (wrsiz > rdsiz)
		tsxx::exceptions::invalid_argument(__FILE__, __LINE__);

	if (rdsiz > wrsiz)
		rdsiz = wrsiz;

	for (cp = static_cast<const uint8_t *>(wrp); wrsiz > 0; cp++, wrsiz--)
		data.write(*cp);

	cs.set();

	tx_bit.set();
	FIXME(); while (busy_bit.get());
	for (p = static_cast<uint8_t *>(rdp); rdsiz > 0; p++, rdsiz--)
		*p = data.read();
	tx_bit.unset();

	cs.unset();
}
