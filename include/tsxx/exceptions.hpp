#if !defined(_TSXX_EXCEPTIONS_HPP_)
#define _TSXX_EXCEPTIONS_HPP_

#include <exception>
#include <sstream>
#include <string>

namespace tsxx
{
namespace exceptions
{

class
exception
: public std::exception
{
public:
	exception()
	{
	}

};

class
invalid_argument
: public exception
{
public:
	invalid_argument()
		: message("invalid argument")
	{
	}

	invalid_argument(const char *file, unsigned int line)
		: message("invalid argument")
	{
		std::ostringstream os;
		os << message << " at " << file << ":" << line;
		string = os.str();

		message = string.c_str();
	}

	virtual
	~invalid_argument() throw()
	{
	}

	virtual const char *
	what() const throw()
	{
		return message;
	}

private:
	const char *message;
	std::string string;

};

class
invalid_state
: public exception
{
public:
	invalid_state()
		: message("invalid state")
	{
	}

	virtual const char *
	what() const throw()
	{
		return message;
	}

private:
	const char * const message;

};

class
not_enough_memory
: public exception
{
public:
	not_enough_memory()
		: message("not enough memory")
	{
	}

	virtual const char *
	what() const throw()
	{
		return message;
	}

private:
	const char * const message;

};

class
unknown_error
: public exception
{
public:
	unknown_error(const char *file, unsigned int line)
		: message("unknown error")
	{
		std::ostringstream os;
		os << message << " at " << file << ":" << line;
		string = os.str();

		message = string.c_str();
	}

	virtual
	~unknown_error() throw()
	{
	}

	virtual const char *
	what() const throw()
	{
		return message;
	}

private:
	const char *message;
	std::string string;

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
