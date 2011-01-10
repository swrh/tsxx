#if !defined(_TSXX_TS7300_DEVICES_HPP_)
#define _TSXX_TS7300_DEVICES_HPP_

#include <vector>

#include <tsxx/ports.hpp>

namespace tsxx
{
namespace ts7300
{
namespace devices
{

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
	dio1(tsxx::system::memory &memory)
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
private:
	enum { BASE_ADDR = 0x80840000 };
public:
	lcd(tsxx::system::memory &memory);

	void init();
	void print(std::string str);
	void print(const void *p, std::size_t len);
	void command(uint8_t cmd);
	bool wait();

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
	xdio(tsxx::system::memory &memory, unsigned int n);

	void init();

private:
	enum mode_cfg {
		MODE_DIO = 0x00,
		MODE_EDGEQUADCNTR = 0x01,
		MODE_INPULSTIMER = 0x02,
		MODE_PWM = 0x03,
		UNINITIALIZED,
	} mode;

	void read_conf();
	void write_conf();

	// Operation mode setting methods.
public:
	void set_mode_dio();

	// DIO methods and variables.
public:
	/**
	 * Returns the DIO port.
	 */
	tsxx::ports::dioport<tsxx::ports::port8> &get_dio();

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
	spi(tsxx::system::memory &memory);

public:
	void init();

	bool add_chip(unsigned int id, tsxx::interfaces::binport &cs);

	bool writenread(unsigned int id, std::vector<uint8_t> &rw_data);
	bool writenread(unsigned int id, const std::vector<uint8_t> &write_data, std::vector<uint8_t> &read_data);

protected:
	std::map<unsigned int, tsxx::interfaces::binport &> chips;

private:
	/// SPI registers.
	tsxx::ports::port16 ctrl, status, data;
	tsxx::ports::bport16 tx_bit, busy_bit, inp_bit;

};

}
}
}

#endif // !defined(_TSXX_TS7300_DEVICES_HPP_)
