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
