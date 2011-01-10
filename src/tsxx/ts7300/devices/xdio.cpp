#include <errno.h>

#include <tsxx/ts7300/devices.hpp>

using tsxx::ts7300::devices::xdio;

xdio::xdio(tsxx::system::memory &memory, unsigned int n)
	: conf(memory.get_region(BASE_ADDR + n * 4 + 0)),
	reg1(memory.get_region(BASE_ADDR + n * 4 + 1)),
	reg2(memory.get_region(BASE_ADDR + n * 4 + 2)),
	reg3(memory.get_region(BASE_ADDR + n * 4 + 3)),
	dio_port(reg1, reg2)
{
	if (n > 1)
		throw tsxx::exceptions::stdio_error(EINVAL); // FIXME use the correct exception

	mode = UNINITIALIZED;
}

void
xdio::init()
{
	read_conf();
}

void
xdio::read_conf()
{
	tsxx::ports::port8::word_type c = conf.read();
	mode = static_cast<mode_cfg>((c >> 6) & 0x03);
}

void
xdio::write_conf()
{
	switch (mode) {
	case MODE_DIO:
	case MODE_EDGEQUADCNTR:
	case MODE_INPULSTIMER:
	case MODE_PWM:
		break;
	case UNINITIALIZED:
	default:
		throw tsxx::exceptions::stdio_error(EINVAL); // FIXME use the correct exception
	}

	conf.write(mode << 6);
}

void
xdio::set_mode_dio()
{
	mode = MODE_DIO;
	write_conf();
}

tsxx::ports::dioport<tsxx::ports::port8> &
xdio::get_dio()
{
	if (mode != MODE_DIO)
		throw tsxx::exceptions::stdio_error(EINVAL); // FIXME use the correct exception
	return dio_port;
}
