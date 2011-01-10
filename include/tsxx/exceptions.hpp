#if !defined(_TSXX_EXCEPTIONS_HPP_)
#define _TSXX_EXCEPTIONS_HPP_

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
	stdio_error(int number) throw()
		: number(number)
	{
		description = ::strerror(number);
	}

	~stdio_error() throw()
	{
	}

	virtual const char *
	what() const throw()
	{
		return description.c_str();
	}

private:
	const int number;
	std::string description;

};

}
}

#endif // !defined(_TSXX_EXCEPTIONS_HPP_)
