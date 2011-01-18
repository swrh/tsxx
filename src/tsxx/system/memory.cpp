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

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <tsxx/system.hpp>

using tsxx::system::memory;
using tsxx::system::memory_region_window;

memory::memory()
{
	if (getpagesize() < 0)
		throw tsxx::exceptions::stdio_error(errno);
	region_size = static_cast<std::size_t>(getpagesize());
}

std::size_t
memory::get_region_size() const
{
	return region_size;
}

bool
memory::open()
{
	if (is_opened())
		return true;

	// We must use O_SYNC argument or else we WILL crash or get
	// random access while acessing registers.
	int val = ::open("/dev/mem", O_RDWR | O_SYNC);
	if (val == -1)
		return false;

	fd.reset(new file_descriptor(val));

	return true;
}

void
memory::try_close()
{
	if (!is_opened())
		return;

	memory_regions.clear();
}

bool
memory::is_opened()
{
	if (fd.get() == NULL)
		return false;
	return fd->is_valid();
}

memory_region_window
memory::get_region(off_t address)
{
	if (!is_opened())
		throw tsxx::exceptions::stdio_error(EBADF);

	off_t offset = address % get_region_size();
	address -= offset;

	std::map<off_t, memory_region_ptr>::iterator it = memory_regions.find(address);
	if (it != memory_regions.end())
		return memory_region_window(it->second, offset);

	memory_region_ptr region(new memory_region(fd));
	if (!region->map(get_region_size(), address))
		throw tsxx::exceptions::stdio_error(errno);

	memory_regions.insert(std::pair<off_t, memory_region_ptr>(address, region));

	return memory_region_window(region, offset);
}
