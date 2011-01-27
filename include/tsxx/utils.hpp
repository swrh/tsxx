#if !defined(_TSXX_UTILS_HPP_)
#define _TSXX_UTILS_HPP_

#include <tsxx/system.hpp>
#include <tsxx/ports.hpp>

namespace
tsxx
{
// TODO This namespace should be rethought.
namespace
utils
{

class
model
{
private:
	model();

public:
	enum BoardModel {
		UNKNOWN = 0,
		TS7300,
	};

	static enum BoardModel
	identify_board(tsxx::system::memory &mem)
	{
		tsxx::ports::port16 pldrev(mem.get_region(0x23400000));
		tsxx::ports::port16 model(mem.get_region(0x22000000));

		if ((model.read() & 0x07) == 0x03 && (pldrev.read() & 0x07) == 0x03)
			return TS7300;

		return UNKNOWN;
	}

};

class
hw
{
public:
	static float
	uptime(tsxx::system::memory &mem)
	{
		enum model::BoardModel model = model::identify_board(mem);
		uint32_t persec, n;

		if (model == model::TS7300) {
			tsxx::ports::port32 reg(mem.get_region(0x12000004));

			n = reg.read();
			persec = 14745600;
		} else {
			tsxx::ports::port32 reg(mem.get_region(0x80810060));

			n = reg.read();
			persec = 983040;
		}

		return static_cast<float>(n) / persec;
	}
};

class
cpu
{
private:
	cpu();

public:
	// TODO Calibrate this function.
	/**
	 * @warning This function hasn't been calibrated yet.
	 */
	static inline void
	nssleep(unsigned int ns)
	{
		volatile unsigned int loop = ns * 5;
		asm volatile (
			"1:\n"
			"subs %1, %1, #1;\n"
			"bne 1b;\n"
			: "=r" ((loop)) : "r" ((loop))
		);
	}

};

}
}

#endif
