#include <tsxx/system.hpp>

using tsxx::system::file_descriptor;

file_descriptor::file_descriptor(int fd)
{
	this->fd = fd;
}

file_descriptor::~file_descriptor()
{
	close();
}

int
file_descriptor::get_value() const
{
	return fd;
}

void
file_descriptor::close()
{
	if (!is_valid())
		return;

	::close(fd);
	fd = -1;
}

bool
file_descriptor::is_valid()
{
	return get_value() >= 0;
}
