#pragma once

constexpr int BUFLEN = 1024;

struct Packet
{
	enum class Type
	{
		INVALID,
		LAUNCH_REQUEST,
		LAUNCH_SUCCESS,
		LAUNCH_FAILED,

		REGISTER_REQUEST,
		REGISTER_SUCCESS,
		REGISTER_FAILED,

		DISTRIBUTE_POKEMENS,
		DISTRIBUTE_A_POKEMEN,
		INSERT_A_POKEMEN,

		BATTLE_REQUEST,

		UPDATE_POKEMENS,
		UPDATE_USERS,

		GET_ONLINE_USERS,
		SET_ONLINE_USERS,

		DISCONNECT
	};

	typedef char Data[BUFLEN];

	Type type;
	Data data;

	Packet();
	Packet(const Packet& other);
	Packet(Packet&& other);
	Packet& operator=(const Packet& other);
	Packet& operator=(Packet&& other);
};