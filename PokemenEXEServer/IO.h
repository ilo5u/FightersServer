#pragma once

constexpr int BUFLEN = 2048;

/// <summary>
/// IO包结构
/// +------+------+
/// + 类型 + 数据 +
/// +------+------+
/// </summary>
struct Packet
{
	enum class Type
	{
		INVALID = 0x0000,

		LOGIN_REQUEST = 0x0001,
		LOGIN_SUCCESS = 0x0002,
		LOGIN_FAILED = 0x0003,
		LOGON_REQUEST = 0x0004,
		LOGON_SUCCESS = 0x0005,
		LOGON_FAILED = 0x0006,
		LOGOUT = 0x0007,

		PVE_RESULT = 0x0010,
		PVP_REQUEST = 0x0020,
		PVP_FAILED = 0x0030,
		PVP_BATTLE = 0x0040,
		PVP_RESULT = 0x0050,
		PVP_MESSAGE = 0x0070,
		PVP_ACCEPT = 0x0090,
		PVP_CANCEL = 0x00A0,
		PVP_BUSY = 0x00C0,

		SET_ONLINE_USERS = 0x0100,
		UPDATE_ONLINE_USERS = 0x0200,
		UPDATE_POKEMENS = 0x0300,
		UPDATE_RANKLIST = 0x0400,
		GET_ONLINE_USERS = 0x0500,
		PROMOTE_POKEMEN = 0x0600,
		ADD_POKEMEN = 0x0700,
		SUB_POKEMEN = 0x0800,
		GET_POKEMENS_BY_USER = 0x0900,
		SET_POKEMENS_BY_USER = 0x0A00,
		SET_POKEMENS_OVER = 0x0B00,
		RENEW_RANKLIST = 0x0C00,
		SET_RANKLIST = 0x0D00
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

typedef enum class _OPERATION_TYPE
{
	SEND_POSTED,
	RECV_POSTED,
	NULL_POSTED
} OPERATION_TYPE;

constexpr int DATA_BUFSIZE = 4096;
typedef struct
{
	OVERLAPPED overlapped; // 重叠结构
	WSABUF dataBuf; // 缓冲区对象
	CHAR buffer[DATA_BUFSIZE]; // 缓冲区数组
	OPERATION_TYPE opType;
	DWORD sendBytes;
	DWORD totalBytes;
} PER_IO_OPERATION_DATA, *LPPER_IO_OPERATION_DATA;

typedef struct
{
	SOCKET client;
	SOCKADDR_IN addr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;