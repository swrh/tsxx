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

#if !defined(_TSXX_REGISTERS_HPP_)
#define _TSXX_REGISTERS_HPP_

#include <stdint.h>

#include <tsxx/exceptions.hpp>

namespace tsxx
{
namespace registers
{

// WordReg
class
reg8
{
public:
	typedef uint8_t word_type;

	reg8(void *p);

	// Force data access using the "ldrb" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline uint8_t
	read()
	{
		volatile uint8_t ret;
		asm volatile (
			"ldrb %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	// Force data access using the "strb" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline void
	write(uint8_t dat)
	{
		asm volatile (
			"strb %1, [ %0 ]\n"
			:
			: "r" (address), "r" (dat)
			: "memory"
		);
	}

private:
	const unsigned long address;

};

// WordReg
class
reg16
{
public:
	typedef uint16_t word_type;

	reg16(void *p);

	// Force data access using the "ldrh" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline uint16_t
	read()
	{
		volatile uint16_t ret;
		asm volatile (
			"ldrh %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	// Force data access using the "strh" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline void
	write(uint16_t dat)
	{
		asm volatile (
			"strh %1, [ %0 ]\n"
			:
			: "r" (address), "r" (dat)
			: "memory"
		);
	}

private:
	const unsigned long address;

};

// WordReg
class
reg32
{
public:
	typedef uint32_t word_type;

	reg32(void *p);

	// Force data access using the "ldr" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline uint32_t
	read()
	{
		volatile uint32_t ret;
		asm volatile (
			"ldr %0, [ %1 ]\n"
			: "=r" (ret)
			: "r" (address)
			: "memory"
		);
		return ret;
	}

	// Force data access using the "str" instruction -- don't rely on
	// compiler optimization by using pointers.
	inline void
	write(uint32_t dat)
	{
		asm volatile (
			"str %1, [ %0 ]\n"
			:
			: "r" (address), "r" (dat)
			: "memory"
		);
	}

private:
	const unsigned long address;

};

}
}

#endif // !defined(_TSXX_REGISTERS_HPP_)
