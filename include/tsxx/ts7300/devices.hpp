#if !defined(_TSXX_TS7300_DEVICES_HPP_)
#define _TSXX_TS7300_DEVICES_HPP_

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
#define LCD_CMD_ROW0			(0x80 | 0x00)
#define LCD_CMD_ROW1			(0x80 | 0x40)
#define LCD_CMD_ROW2			(0x80 | 0x14)
#define LCD_CMD_ROW3			(0x80 | 0x54)
#define LCD_CMD_ROW(x)			(x == 1 ? LCD_CMD_ROW1 : x == 2 ? LCD_CMD_ROW2 : x == 3 ? LCD_CMD_ROW3 : LCD_CMD_ROW0)

#define LCD_CMD_CLEAR			0x0001
#define LCD_CMD_HOME			0x0002

#define LCD_CMD_ENTRY			0x0004
#define LCD_BIT_ENTRY_DIR_RIGHT		0x0002
#define LCD_BIT_ENTRY_DIR_LEFT		0x0000

#define LCD_CMD_CTRL			0x0008
#define LCD_BIT_CTRL_BLNK_OFF		0x0000
#define LCD_BIT_CTRL_BLNK_ON		0x0001
#define LCD_BIT_CTRL_CUR_OFF		0x0000
#define LCD_BIT_CTRL_CUR_ON		0x0002
#define LCD_BIT_CTRL_DSP_OFF		0x0000
#define LCD_BIT_CTRL_DSP_ON		0x0004

#define LCD_CMD_FNSET			0x0020
#define LCD_BIT_FNSET_FONT_5x8		0x0000
#define LCD_BIT_FNSET_FONT_5x11		0x0004
#define LCD_BIT_FNSET_NLINES_1LIN	0x0000
#define LCD_BIT_FNSET_NLINES_2LIN	0x0008
#define LCD_BIT_FNSET_DATLEN_4BIT	0x0000
#define LCD_BIT_FNSET_DATLEN_8BIT	0x0010

#define LCD_CMD_CGRAM			0x0040
#define LCD_CMD_DDRAM			0x0080
#define LCD_CMD_RDSTAT			0x0100
#define LCD_CMD_WRDATA			0x0200
#define LCD_CMD_RDDATA			0x0300
	// TODO not finished yet
private:
	enum { BASE_ADDR = 0x80840000 };
public:
	lcd(tsxx::system::memory &memory)
		: data(tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x00)), tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x10))),
		data7(tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x08)), tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x18))),
		data_mask(0x7f), data7_mask(0x01),
		data_bit_busy(0x80),
		ctrl(tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x40)), tsxx::ports::port8(memory.get_region(BASE_ADDR + 0x44))),
		ctrl_bit_en(0x08), ctrl_bit_rs(0x10), ctrl_bit_wr(0x20)
	{
	}

	void
	init()
	{
		// Set LCD control pins as outputs.
		ctrl.set_dir(ctrl.get_dir() | ctrl_bit_en | ctrl_bit_rs | ctrl_bit_wr);

		// De-assert EN and RS.
		ctrl.write(ctrl.read() & ~(ctrl_bit_rs | ctrl_bit_en));

		usleep(15000);
		command(LCD_CMD_FNSET |
		    LCD_BIT_FNSET_FONT_5x8 |
		    LCD_BIT_FNSET_NLINES_2LIN |
		    LCD_BIT_FNSET_DATLEN_8BIT);
		usleep(4100);
		command(LCD_CMD_FNSET |
		    LCD_BIT_FNSET_FONT_5x8 |
		    LCD_BIT_FNSET_NLINES_2LIN |
		    LCD_BIT_FNSET_DATLEN_8BIT);
		usleep(100);
		command(LCD_CMD_FNSET |
		    LCD_BIT_FNSET_FONT_5x8 |
		    LCD_BIT_FNSET_NLINES_2LIN |
		    LCD_BIT_FNSET_DATLEN_8BIT);
		usleep(39);
		wait();

		command(LCD_CMD_FNSET |
		    LCD_BIT_FNSET_FONT_5x8 |
		    LCD_BIT_FNSET_NLINES_2LIN |
		    LCD_BIT_FNSET_DATLEN_8BIT);
		wait();

		command(LCD_CMD_ENTRY |
		    LCD_BIT_ENTRY_DIR_RIGHT);
		usleep(39);
		wait();

		command(LCD_CMD_CLEAR);
		usleep(1530);
		wait();

		command(LCD_CMD_CTRL |
		    LCD_BIT_CTRL_BLNK_OFF |
		    LCD_BIT_CTRL_CUR_OFF |
		    LCD_BIT_CTRL_DSP_ON);
		wait();

		command(LCD_CMD_HOME);
		usleep(1530);
		wait();
	}

	void
	print(std::string str)
	{
		print(str.c_str(), str.length());
	}

	void
	print(const void *p, std::size_t len)
	{
		// Set LCD data pins as outputs.
		data.set_dir(data.get_dir() | data_mask);
		data7.set_dir(data7.get_dir() | data7_mask);

		tsxx::ports::port8::word_type c = ctrl.read();

		for (const uint8_t *s = static_cast<const uint8_t *>(p); len-- > 0; s++) {
			// Write data to be sent.
			data.write((data.read() & ~data_mask) | (*s & data_mask));
			data7.write((data7.read() & ~data7_mask) | ((*s >> 7) & data7_mask));

			// Assert WR and RS.
			c = (c & ~ctrl_bit_wr) | ctrl_bit_rs;
			ctrl.write(c);

			// Sleep 100ns at least.
			tsxx::system::nssleep(100);

			// Assert EN.
			c |= ctrl_bit_en;
			ctrl.write(c);

			// Sleep 300ns at least.
			tsxx::system::nssleep(300);

			// De-assert EN.
			c &= ~ctrl_bit_en;
			ctrl.write(c);

			// Sleep 200ns at least.
			tsxx::system::nssleep(200);
		}
	}

	void
	command(uint8_t cmd)
	{
		tsxx::ports::port8::word_type c = ctrl.read();

		// Set LCD data pins as outputs.
		data.set_dir(data.get_dir() | data_mask);
		data7.set_dir(data7.get_dir() | data7_mask);

		// 20110110 XXX Is this block really necessary?
#if 0
		// Assert RS.
		c |= ctrl_bit_rs;
		ctrl.write(c);
#endif

		// Write data to be sent.
		data.write((data.read() & ~data_mask) | (cmd & data_mask));
		data7.write((data7.read() & ~data7_mask) | ((cmd >> 7) & data7_mask));

		c &= ~(ctrl_bit_rs | ctrl_bit_wr); // de-assert RS, assert WR
		ctrl.write(c);

		// Sleep 100ns at least.
		tsxx::system::nssleep(100);

		// step 3, assert EN
		c |= ctrl_bit_en;
		ctrl.write(c);

		// Sleep 300ns at least.
		tsxx::system::nssleep(300);

		// step 5, de-assert EN
		c &= ~ctrl_bit_en; // de-assert EN
		ctrl.write(c);

		// Sleep 200ns at least.
		tsxx::system::nssleep(200);
	}

	bool
	wait()
	{
		unsigned int tries;
		tsxx::ports::port8::word_type d, c = ctrl.read();

		// Set LCD data pins as inputs.
		data.set_dir(data.get_dir() & ~data_mask);
		data7.set_dir(data7.get_dir() & ~data7_mask);

		tries = 0;
		do {
			// De-assert RS and WR.
			c = (c | ctrl_bit_wr) & ~ctrl_bit_rs;
			ctrl.write(c);

			// Sleep 100ns at least.
			tsxx::system::nssleep(100);

			// Assert EN.
			c |= ctrl_bit_en;
			ctrl.write(c);

			// Sleep 300ns at least.
			tsxx::system::nssleep(300);

			// De-assert EN, read result

			d = (data.read() & data_mask) | ((data7.read() & data7_mask) << 7);

			// De-assert EN.
			c &= ~ctrl_bit_en;
			ctrl.write(c);

			// Sleep 200ns at least.
			tsxx::system::nssleep(200);
		} while ((d & data_bit_busy) != 0 && tries++ < 1024);

		return (d & data_bit_busy) == 0;
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
	xdio(tsxx::system::memory &memory, unsigned int n)
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
	init()
	{
		read_conf();
	}

private:
	enum mode_cfg {
		MODE_DIO = 0x00,
		MODE_EDGEQUADCNTR = 0x01,
		MODE_INPULSTIMER = 0x02,
		MODE_PWM = 0x03,
		UNINITIALIZED,
	} mode;

	void
	read_conf()
	{
		tsxx::ports::port8::word_type c = conf.read();
		mode = static_cast<mode_cfg>((c >> 6) & 0x03);
	}

	void
	write_conf()
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

	// Operation mode setting methods.
public:
	void
	set_mode_dio()
	{
		mode = MODE_DIO;
		write_conf();
	}

	// DIO methods and variables.
public:
	/**
	 * Returns the DIO port.
	 */
	tsxx::ports::dioport<tsxx::ports::port8> &
	get_dio()
	{
		if (mode != MODE_DIO)
			throw tsxx::exceptions::stdio_error(EINVAL); // FIXME use the correct exception
		return dio_port;
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
	spi(tsxx::system::memory &memory)
		: ctrl(memory.get_region(BASE_ADDR + 0x04)),
		status(memory.get_region(BASE_ADDR + 0x0c)),
		data(memory.get_region(BASE_ADDR + 0x08)),
		tx_bit(ctrl, 4),
		busy_bit(status, 4),
		inp_bit(status, 2)
	{
	}

public:
	bool
	add_chip(unsigned int id, tsxx::interfaces::binport &cs)
	{
		if (chips.find(id) != chips.end())
			return false;

		cs.unset();

		chips.insert(std::pair<unsigned int, tsxx::interfaces::binport &>(id, cs));

		return true;
	}

	void
	init()
	{
		tx_bit.set();
		FIXME(); while (busy_bit.get());
		tx_bit.unset();
		FIXME(); while (busy_bit.get()); // Is this really necessary?
		FIXME(); while (inp_bit.get())
			(void)data.read();
	}

	bool
	writenread(unsigned int id, std::vector<uint8_t> &rw_data)
	{
		return writenread(id, rw_data, rw_data);
	}

	bool
	writenread(unsigned int id, const std::vector<uint8_t> &write_data, std::vector<uint8_t> &read_data)
	{
		std::map<unsigned int, tsxx::interfaces::binport &>::iterator chip = chips.find(id);
		if (chip != chips.end())
			return false;

		if (read_data.size() != write_data.size())
			read_data.resize(write_data.size());

		for (std::vector<uint8_t>::const_iterator w = write_data.begin(); w != write_data.end(); w++)
			data.write(*w);

		chip->second.set();

		tx_bit.set();
		FIXME(); while (busy_bit.get());

		chip->second.unset();

		for (std::vector<uint8_t>::iterator w = read_data.begin(); w != read_data.end(); w++)
			*w = data.read();
		tx_bit.unset();

		return true;
	}

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
