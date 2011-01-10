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

}
}

#endif // !defined(_TSXX_INTERFACES_HPP_)
