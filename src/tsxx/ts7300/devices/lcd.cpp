#include <tsxx/ts7300/devices.hpp>

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

using tsxx::ts7300::devices::lcd;

lcd::lcd(tsxx::system::memory &memory)
	: data(memory.get_region(BASE_ADDR + 0x00), memory.get_region(BASE_ADDR + 0x10)),
	data7(memory.get_region(BASE_ADDR + 0x08), memory.get_region(BASE_ADDR + 0x18)),
	data_mask(0x7f), data7_mask(0x01),
	data_bit_busy(0x80),
	ctrl(memory.get_region(BASE_ADDR + 0x40), memory.get_region(BASE_ADDR + 0x44)),
	ctrl_bit_en(0x08), ctrl_bit_rs(0x10), ctrl_bit_wr(0x20)
{
}

void
lcd::init()
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
lcd::print(std::string str)
{
	print(str.c_str(), str.length());
}

void
lcd::print(const void *p, std::size_t len)
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
lcd::command(uint8_t cmd)
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
lcd::wait()
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
