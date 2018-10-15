#pragma once
constexpr int NAME_LENGTH = 40;
typedef char USER_NAME[NAME_LENGTH];

constexpr int PASSWORD_LENGTH = 80;
typedef char USER_PASSWORD[PASSWORD_LENGTH];

constexpr int BUFLEN = 1024;

struct Packet
{
	enum class Type
	{
		INVALID,
		LAUNCH_REQUEST,
		LAUNCH_SUCCESS,
		LAUNCH_FAILED
	};

	struct UserInfo
	{
		USER_NAME name;
		USER_PASSWORD password;
	};

	Type type;
	union Data
	{
		UserInfo user_info;
	};
	Data data;

	Packet();
	Packet(const Packet& other);
	Packet(Packet&& other);
	Packet& operator=(const Packet& other);
	Packet& operator=(Packet&& other);
};