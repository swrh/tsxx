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
