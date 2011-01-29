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

#include <errno.h>

#include <tsxx/ts7300/devices.hpp>

using tsxx::ts7300::devices::xdio;

xdio::xdio(tsxx::system::memory &memory, unsigned int n)
	: conf(memory.get_region(BASE_ADDR + n * 4 + 0)),
	reg1(memory.get_region(BASE_ADDR + n * 4 + 1)),
	reg2(memory.get_region(BASE_ADDR + n * 4 + 2)),
	reg3(memory.get_region(BASE_ADDR + n * 4 + 3)),
	dio_port(reg2, reg1)
{
	if (n > 1)
		throw tsxx::exceptions::invalid_argument(__FILE__, __LINE__);

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
		throw tsxx::exceptions::invalid_state();
	}

	conf.write(mode << 6);
}

void
xdio::set_mode_dio()
{
	mode = MODE_DIO;
	write_conf();
}
