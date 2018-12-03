#pragma once

typedef bool        Boolean;
typedef std::string String;
typedef std::thread Thread;
typedef std::mutex  Mutex;
typedef SOCKET      Socket;
typedef SOCKADDR_IN SockaddrIn;
typedef HANDLE      Handle;

typedef std::queue<Packet>   Packets;
typedef Pokemen::BattleStage BattleStage;
typedef Pokemen::PokemenType PokemenType;

typedef std::list<Pokemen::Pokemen> Pokemens;

class Server;
class User
{
	friend Server;
public:
	enum class BattleType
	{
		PVP
	};

public:
	User(const Socket& client, const SockaddrIn& clientAddr);
	~User();

public:
	void   InsertAPokemen(const String& info);

public:
	ULONG  GetUserID() const;
	void   SetUsername(const String& username);
	String GetUsername() const;

private:
	/* 连接信息 */
	const Socket m_client;
	const SockaddrIn m_clientAddr;
	LPPER_IO_OPERATION_DATA m_io;

	/* 用户信息 */
	String m_username;
	Pokemens m_pokemens;
	int m_rounds;
	int m_wins;
};