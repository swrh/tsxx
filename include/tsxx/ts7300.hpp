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
	board(tsxx::system::memory &mem);

	void init();

	ts7300::devices::xdio &get_xdio1();
	ts7300::devices::xdio &get_xdio2();
	ts7300::devices::dio1 &get_dio1();
	ts7300::devices::lcd &get_lcd();
	ts7300::devices::spi &get_spi();

private:
	tsxx::system::memory &memory;

	ts7300::devices::xdio xdio1;
	ts7300::devices::xdio xdio2;
	ts7300::devices::dio1 dio1;
	ts7300::devices::lcd lcd;
	ts7300::devices::spi spi;

};

}
}

#endif // !defined(_TSXX_TS7300_HPP_)
