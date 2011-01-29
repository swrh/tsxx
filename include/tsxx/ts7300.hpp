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
