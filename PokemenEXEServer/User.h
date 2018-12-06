#pragma once

typedef bool        Boolean;
typedef std::string String;
typedef std::thread Thread;
typedef std::mutex  Mutex;
typedef SOCKET      Socket;
typedef SOCKADDR_IN SockaddrIn;
typedef HANDLE      Handle;

typedef std::queue<Packet>   Packets;
typedef Pokemen::PokemenType PokemenType;

typedef std::list<Pokemen::Pokemen> Pokemens;

class Server;

struct User
{
	/* 用户信息 */
	String username;
	int numberOfPokemens = 0;
	int rounds = 0;
	int wins = 0;
	int tops = 0;
};

class OnlineUser : private User
{
	friend Server;
public:

public:
	OnlineUser(const Socket& client, const SockaddrIn& clientAddr);
	~OnlineUser();

public:
	void   InsertAPokemen(const String& info);

public:
	ULONG  GetUserID() const;
	void   SetUsername(const String& username);
	String GetUsername() const;

public:
	int ReadIOCounter();
	void IncIOCounter();
	void DecIOCounter();

private:
	/* 连接信息 */
	const Socket m_client;
	const SockaddrIn m_clientAddr;

	Mutex m_ioLocker;
	int   m_ioHandlerCount;

	Pokemens m_pokemens;
	bool     m_needRemove;
};