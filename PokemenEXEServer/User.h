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

/// <summary>
/// 用户数据类型
/// </summary>
struct User
{
	String username;
	int numberOfPokemens = 0;
	int rounds = 0;
	int wins = 0;
	int tops = 0;
};

/// <summary>
/// 在线用户类型
/// 维护用户的连接
/// </summary>
class OnlineUser : private User
{
	friend Server;
public:

public:
	OnlineUser(const Socket& client, const SockaddrIn& clientAddr);
	~OnlineUser();

public:
	void   InsertAPokemen(const String& info);
	String PokemenAt(int pokemenId) const;

public:
	ULONG  GetUserID() const;
	void   SetUsername(const String& username);
	String GetUsername() const;
	String GetOpponent() const;

public:
	/* IO重叠资源控制 */
	int  ReadIORecvCounter();
	void IncIORecvCounter();
	void DecIORecvCounter();

	int  ReadIOSendCounter();
	void IncIOSendCounter();
	void DecIOSendCounter();

private:
	/* 连接信息 */
	const Socket m_client;
	const SockaddrIn m_clientAddr;

	/* IO重叠资源计数 */
	Mutex m_ioSendLocker;
	int   m_ioSendCount;
	Mutex m_ioRecvLocker;
	int   m_ioRecvCount;

	Mutex m_pokemenLocker;
	Pokemens m_pokemens;
	bool     m_needRemove;
	String   m_opponent; // 当前在线对战的敌方用户名
};