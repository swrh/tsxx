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
#include <sys/mman.h>

#include <tsxx/system.hpp>

using tsxx::system::memory_region;

memory_region::memory_region(file_descriptor_ptr fd)
{
	if (fd.get() == NULL || !fd->is_valid())
		throw tsxx::exceptions::stdio_error(EINVAL);

	this->fd = fd;
	pointer = MAP_FAILED;
	length = 0;
}

memory_region::~memory_region()
{
	unmap();
}

bool
memory_region::map(std::size_t len, off_t offset)
{
	if (pointer != MAP_FAILED) {
		errno = EBADF;
		return false;
	}

	pointer = ::mmap(0, len, PROT_READ|PROT_WRITE, MAP_SHARED, fd->get_value(), offset);
	if (pointer == MAP_FAILED)
		return false;

	length = len;

	return true;
}

void
memory_region::unmap()
{
	if (pointer == MAP_FAILED)
		return;

	(void)::munmap(pointer, length);

	pointer = MAP_FAILED;
	length = 0;
}

void *
memory_region::get_pointer()
{
	if (pointer == MAP_FAILED)
		return NULL;
	return pointer;
}
