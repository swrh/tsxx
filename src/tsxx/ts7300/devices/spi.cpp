#include <iostream>

#include <tsxx/ts7300/devices.hpp>

using tsxx::ts7300::devices::spi;

spi::spi(tsxx::system::memory &memory)
	: ctrl(memory.get_region(BASE_ADDR + 0x04)),
	status(memory.get_region(BASE_ADDR + 0x0c)),
	data(memory.get_region(BASE_ADDR + 0x08)),
	tx_bit(ctrl, 4),
	busy_bit(status, 4),
	inp_bit(status, 2)
{
}

bool
spi::add_chip(unsigned int id, tsxx::interfaces::binport &cs)
{
	if (chips.find(id) != chips.end())
		return false;

	cs.unset();

	chips.insert(std::pair<unsigned int, tsxx::interfaces::binport &>(id, cs));

	return true;
}

void
spi::init()
{
	tx_bit.set();
	FIXME(); while (busy_bit.get());
	tx_bit.unset();
	FIXME(); while (busy_bit.get()); // Is this really necessary?
	FIXME(); while (inp_bit.get())
		(void)data.read();
}

bool
spi::writenread(unsigned int id, std::vector<uint8_t> &rw_data)
{
	return writenread(id, rw_data, rw_data);
}

bool
spi::writenread(unsigned int id, const std::vector<uint8_t> &write_data, std::vector<uint8_t> &read_data)
{
	std::map<unsigned int, tsxx::interfaces::binport &>::iterator chip = chips.find(id);
	if (chip != chips.end())
		return false;

	if (read_data.size() != write_data.size())
		read_data.resize(write_data.size());

	for (std::vector<uint8_t>::const_iterator w = write_data.begin(); w != write_data.end(); w++)
		data.write(*w);

	chip->second.set();

	tx_bit.set();
	FIXME(); while (busy_bit.get());

	chip->second.unset();

	for (std::vector<uint8_t>::iterator w = read_data.begin(); w != read_data.end(); w++)
		*w = data.read();
	tx_bit.unset();

	return true;
}
