#include "stdafx.h"
#include "IO.h"

Packet::Packet()
{
	std::memset(&data, 0x0, sizeof(data));
}

Packet::Packet(const Packet& other) :
	type(other.type)
{
	switch (type)
	{
	case Type::LAUNCH_REQUEST:
	case Type::LAUNCH_SUCCESS:
	case Type::LAUNCH_FAILED:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		break;

	default:
		break;
	}
}

Packet::Packet(Packet&& other) :
	type(other.type)
{
	switch (type)
	{
	case Type::LAUNCH_REQUEST:
	case Type::LAUNCH_SUCCESS:
	case Type::LAUNCH_FAILED:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		break;

	default:
		break;
	}
}

Packet& Packet::operator=(const Packet & other)
{
	type = other.type;
	switch (type)
	{
	case Type::LAUNCH_REQUEST:
	case Type::LAUNCH_SUCCESS:
	case Type::LAUNCH_FAILED:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		break;

	default:
		break;
	}
	return *this;
}

Packet& Packet::operator=(Packet && other)
{
	type = other.type;
	switch (type)
	{
	case Type::LAUNCH_REQUEST:
	case Type::LAUNCH_SUCCESS:
	case Type::LAUNCH_FAILED:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		break;

	default:
		break;
	}
	return *this;
}
