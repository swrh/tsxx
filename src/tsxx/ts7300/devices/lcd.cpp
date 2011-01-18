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

#include <tsxx/ts7300/devices.hpp>

#define LCD_CMD_ROW0			(0x80 | 0x00)
#define LCD_CMD_ROW1			(0x80 | 0x40)
#define LCD_CMD_ROW2			(0x80 | 0x14)
#define LCD_CMD_ROW3			(0x80 | 0x54)
#define LCD_CMD_ROW(x)			(x == 1 ? LCD_CMD_ROW1 : x == 2 ? LCD_CMD_ROW2 : x == 3 ? LCD_CMD_ROW3 : LCD_CMD_ROW0)

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
