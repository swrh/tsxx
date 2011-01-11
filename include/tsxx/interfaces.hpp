#if !defined(_TSXX_INTERFACES_HPP_)
#define _TSXX_INTERFACES_HPP_

namespace tsxx
{
namespace interfaces
{

class
binport
{
public:
	virtual void set() = 0;
	virtual void unset() = 0;

};

class
triggerport
{
public:
	triggerport(binport &port, bool updown = false)
		: bit(port), updown(updown)
	{
	}

	void
	fire()
	{
		if (updown) {
			bit.set();
			bit.unset();
		} else {
			bit.unset();
			bit.set();
		}
	}

private:
	binport &bit;
	bool updown;

};


}
}

#endif // !defined(_TSXX_INTERFACES_HPP_)
