#include "stdafx.h"
#include "IO.h"

Packet::Packet() :
	type(Packet::Type::INVALID)
{
	std::memset(data, 0x0, sizeof(data));
}

Packet::Packet(const Packet& other) :
	type(other.type)
{
	std::strncpy(data, other.data, BUFLEN);
}

Packet::Packet(Packet&& other) :
	type(other.type)
{
	std::strncpy(data, other.data, BUFLEN);
}

Packet& Packet::operator=(const Packet& other)
{
	type = other.type;
	std::strncpy(data, other.data, BUFLEN);
	return *this;
}

Packet& Packet::operator=(Packet&& other)
{
	type = other.type;
	std::strncpy(data, other.data, BUFLEN);
	return *this;
}