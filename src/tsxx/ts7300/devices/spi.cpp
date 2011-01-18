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
		tsxx::exceptions::invalid_argument();

	if (rdsiz > wrsiz)
		rdsiz = wrsiz;

	for (cp = static_cast<const uint8_t *>(wrp); wrsiz > 0; cp++, wrsiz--)
		data.write(*cp);

	cs.set();

	tx_bit.set();
	FIXME(); while (busy_bit.get());

	cs.unset();

	for (p = static_cast<uint8_t *>(rdp); rdsiz > 0; p++, rdsiz--)
		*p = data.read();
	tx_bit.unset();
}
