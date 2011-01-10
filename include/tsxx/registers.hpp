#if !defined(_TSXX_REGISTERS_HPP_)
#define _TSXX_REGISTERS_HPP_

#include <tsxx/exceptions.hpp>

namespace tsxx
{
namespace registers
{

// WordReg
class
reg8bits
{
public:
	typedef uint8_t word_type;

	reg8bits(void *p)
		: address(reinterpret_cast<unsigned long>(p))
	{
		if (p == NULL)
			throw tsxx::exceptions::stdio_error(EINVAL);
	}

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
reg16bits
{
public:
	typedef uint16_t word_type;

	reg16bits(void *p)
		: address(reinterpret_cast<unsigned long>(p))
	{
		if (p == NULL)
			throw tsxx::exceptions::stdio_error(EINVAL);
	}

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
reg32bits
{
public:
	typedef uint32_t word_type;

	reg32bits(void *p)
		: address(reinterpret_cast<unsigned long>(p))
	{
		if (p == NULL)
			throw tsxx::exceptions::stdio_error(EINVAL);
	}

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
