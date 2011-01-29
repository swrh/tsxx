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

#if !defined(_TSXX_SYSTEM_HPP_)
#define _TSXX_SYSTEM_HPP_

#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <tsxx/exceptions.hpp>

#define LOG(x) (std::clog << __FILE__ << ":" << __LINE__ << ": " << (x) << std::endl)
#define FIXME() LOG("FIXME")
#define TODO() LOG("TODO")
#define XXX() LOG("XXX")

namespace tsxx
{
namespace system
{

class
file_descriptor
: private boost::noncopyable
{
public:
	file_descriptor(int fd = -1);
	~file_descriptor();

	int get_value() const;
	void close();
	bool is_valid();

private:
	int fd;

};
typedef boost::shared_ptr<file_descriptor> file_descriptor_ptr;

class
memory_region
: private boost::noncopyable
{
public:
	memory_region(file_descriptor_ptr fd);
	~memory_region();

	bool map(std::size_t len, off_t offset);
	void unmap();

	void *get_pointer();

private:
	file_descriptor_ptr fd;
	void *pointer;
	std::size_t length;

};
typedef boost::shared_ptr<memory_region> memory_region_ptr;

class
memory_region_window
{
public:
	memory_region_window(memory_region_ptr reg, off_t off);

	void *get_pointer();

private:
	memory_region_ptr region;
	off_t offset;

};

class
memory
: private boost::noncopyable // XXX check if this is really needed
{
public:
	memory();

	std::size_t get_region_size() const;

	bool is_opened();
	bool open();
	void try_close();

	memory_region_window get_region(off_t address);

private:
	file_descriptor_ptr fd;
	std::map<off_t, memory_region_ptr> memory_regions;
	std::size_t region_size;

};

}
}

#endif // !defined(_TSXX_SYSTEM_HPP_)
