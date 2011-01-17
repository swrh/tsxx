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
invalid_argument
: public exception
{
};

class
invalid_state
: public exception
{
};

class
not_enough_memory
: public exception
{
};

class
unknown_error
: public exception
{
public:
	unknown_error(const char *_file, unsigned int _line)
		: file(_file), line(_line)
	{
	}

	virtual ~unknown_error() throw()
	{
	}

	virtual const char *
	what() const throw()
	{
		return "TODO";
	}

private:
	const char *file;
	unsigned int line;

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
