// Copyright (c) 2011 Fernando Silveira <fsilveira@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation
// are those of the authors and should not be interpreted as representing
// official policies, either expressed or implied, of Fernando Silveira.

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
{
private:
	enum { BASE_ADDR = 0x80840000 };
public:
	dio1(tsxx::system::memory &memory)
		: dio17_mask(0xfb),
		dio17(memory.get_region(BASE_ADDR + 0x04), memory.get_region(BASE_ADDR + 0x14)),
		dio8_mask(0x04),
		dio8(memory.get_region(BASE_ADDR + 0x30), memory.get_region(BASE_ADDR + 0x34))
	{
	}

public:
	typedef tsxx::ports::dioport<tsxx::ports::port8> port_type;

	// WordPort
public:
	typedef port_type::word_type word_type;

	inline void
	write(word_type word)
	{
		dio17.write(word & dio17_mask);
		dio8.write((word & dio8_mask) >> 1);
	}

	inline word_type
	read()
	{
		return (dio17.read() & dio17_mask) | ((dio8.read() & dio8_mask) << 1);
	}

public:
	inline void
	set_dir(word_type word)
	{
		dio17.set_dir(word & dio17_mask);
		dio8.set_dir((word & dio8_mask) >> 1);
	}

	inline word_type
	get_dir()
	{
		return (dio17.get_dir() & dio17_mask) | ((dio8.get_dir() & dio8_mask) << 1);
	}

private:
	const port_type::word_type dio17_mask;
	port_type dio17;

	const port_type::word_type dio8_mask;
	port_type dio8;

};

/**
 * LCD port class.
 */
class
lcd
{
private:
	enum { BASE_ADDR = 0x80840000 };

	enum {
		LCD_CMD_CLEAR =			0x01,

		LCD_CMD_HOME =			0x02,

		LCD_CMD_ENTRY =			0x04,
		LCD_BIT_ENTRY_DIR_RIGHT =	0x02,
		LCD_BIT_ENTRY_DIR_LEFT =	0x00,

		LCD_CMD_CTRL =			0x08,
		LCD_BIT_CTRL_BLNK_OFF =		0x00,
		LCD_BIT_CTRL_BLNK_ON =		0x01,
		LCD_BIT_CTRL_CUR_OFF =		0x00,
		LCD_BIT_CTRL_CUR_ON =		0x02,
		LCD_BIT_CTRL_DSP_OFF =		0x00,
		LCD_BIT_CTRL_DSP_ON =		0x04,

		LCD_CMD_FNSET =			0x20,
		LCD_BIT_FNSET_FONT_5x8 =	0x00,
		LCD_BIT_FNSET_FONT_5x11 =	0x04,
		LCD_BIT_FNSET_NLINES_1LIN =	0x00,
		LCD_BIT_FNSET_NLINES_2LIN =	0x08,
		LCD_BIT_FNSET_DATLEN_4BIT =	0x00,
		LCD_BIT_FNSET_DATLEN_8BIT =	0x10,

		LCD_CMD_CGRAM =			0x40,

		LCD_CMD_DDRAM =			0x80,
		LCD_VAL_DDRAM_ROW0 =		0x00,
		LCD_VAL_DDRAM_ROW1 =		0x40,
		LCD_VAL_DDRAM_ROW2 =		0x14,
		LCD_VAL_DDRAM_ROW3 =		0x54,
	};

public:
	lcd(tsxx::system::memory &memory);

	void init();
	void print(std::string str);
	void print(const void *p, std::size_t len);
	void command(uint8_t cmd);
	bool wait();

public:
	void
	clear()
	{
		command(LCD_CMD_CLEAR);
		usleep(1530);
	}

	void
	home()
	{
		command(LCD_CMD_HOME);
		usleep(1530);
	}

	void
	entry_mode(bool right)
	{
		command(LCD_CMD_ENTRY | (right ? LCD_BIT_ENTRY_DIR_RIGHT : LCD_BIT_ENTRY_DIR_LEFT));
		usleep(39);
	}

	void
	control(bool display_on, bool cursor_on, bool blink_on)
	{
		command(LCD_CMD_CTRL |
				(display_on ? LCD_BIT_CTRL_DSP_ON : LCD_BIT_CTRL_DSP_OFF) |
				(cursor_on ? LCD_BIT_CTRL_CUR_ON : LCD_BIT_CTRL_CUR_OFF) |
				(blink_on ? LCD_BIT_CTRL_BLNK_ON : LCD_BIT_CTRL_BLNK_OFF));
		usleep(39);
	}

	void
	function(bool font_5x8, bool n2lines, bool dat8bit)
	{
		command(LCD_CMD_FNSET |
				(font_5x8 ? LCD_BIT_FNSET_FONT_5x8 : LCD_BIT_FNSET_FONT_5x11) |
				(n2lines ? LCD_BIT_FNSET_NLINES_2LIN : LCD_BIT_FNSET_NLINES_1LIN) |
				(dat8bit ? LCD_BIT_FNSET_DATLEN_8BIT : LCD_BIT_FNSET_DATLEN_4BIT));
		usleep(39);
	}

	void
	ddram(unsigned int x, unsigned int y)
	{
		command(LCD_CMD_DDRAM |
				(y == 0 ? LCD_VAL_DDRAM_ROW0 :
				 y == 1 ? LCD_VAL_DDRAM_ROW1 :
				 y == 2 ? LCD_VAL_DDRAM_ROW2 :
				 LCD_VAL_DDRAM_ROW3) |
				x);
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
	inline tsxx::ports::dioport<tsxx::ports::port8> &
	get_dio()
	{
		return dio_port;
	}

	// WordPort
public:
	typedef tsxx::ports::port8::word_type word_type;

	void
	write(word_type word)
	{
		get_dio().write(word);
	}

	word_type
	read()
	{
		return get_dio().read();
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
	spi(tsxx::system::memory &memory);

public:
	void init();

public:
	void write_read(tsxx::interfaces::binport &cs, const void *wrp, std::size_t wrsiz, void *rdp, std::size_t rdsiz);

	inline void
	write_read(tsxx::interfaces::binport &cs, void *rdwrp, std::size_t rdwrsiz)
	{
		write_read(cs, rdwrp, rdwrsiz, rdwrp, rdwrsiz);
	}

	inline void
	write_read(tsxx::interfaces::binport &cs, std::vector<uint8_t> &rw_data)
	{
		write_read(cs, rw_data, rw_data);
	}

	inline void
	write_read(tsxx::interfaces::binport &cs, const std::vector<uint8_t> &wr_data, std::vector<uint8_t> &read_data)
	{
		if (read_data.size() != wr_data.size())
			read_data.resize(wr_data.size());
		write_read(cs, &wr_data[0], wr_data.size(), &read_data[0], read_data.size());
	}

private:
	/// SPI registers.
	tsxx::ports::port16 ctrl, status, data;
	tsxx::ports::bport16 tx_bit, busy_bit, inp_bit;

};

class
spi_chip
{
public:
	spi_chip(spi &_port, tsxx::interfaces::binport &_cs)
		: port(&_port), cs(_cs)
	{
	}

	inline void
	write_read(void *p, std::size_t siz)
	{
		port->write_read(cs, p, siz);
	}

	inline void
	write_read(std::vector<uint8_t> &rw_data)
	{
		port->write_read(cs, rw_data);
	}

	inline void
	write_read(const std::vector<uint8_t> &wr_data, std::vector<uint8_t> &rd_data)
	{
		port->write_read(cs, wr_data, rd_data);
	}

private:
	spi *port;
	tsxx::interfaces::binport &cs;

};

}
}
}

#endif // !defined(_TSXX_TS7300_DEVICES_HPP_)
