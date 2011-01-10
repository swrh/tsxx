#if !defined(_TSXX_EXCEPTIONS_HPP_)
#define _TSXX_EXCEPTIONS_HPP_

#include <string>
#include <exception>

namespace tsxx
{
namespace exceptions
{

class
exception
: public std::exception
{

};

class
stdio_error
: public exception
{
public:
	stdio_error(int number) throw();
	virtual ~stdio_error() throw();
	virtual const char *what() const throw();

private:
	const int number;
	std::string description;

};

}
}

#endif // !defined(_TSXX_EXCEPTIONS_HPP_)
