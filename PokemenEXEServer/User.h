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
/// �û���������
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
/// �����û�����
/// ά���û�������
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
	/* IO�ص���Դ���� */
	int  ReadIORecvCounter();
	void IncIORecvCounter();
	void DecIORecvCounter();

	int  ReadIOSendCounter();
	void IncIOSendCounter();
	void DecIOSendCounter();

private:
	/* ������Ϣ */
	const Socket m_client;
	const SockaddrIn m_clientAddr;

	/* IO�ص���Դ���� */
	Mutex m_ioSendLocker;
	int   m_ioSendCount;
	Mutex m_ioRecvLocker;
	int   m_ioRecvCount;

	Mutex m_pokemenLocker;
	Pokemens m_pokemens;
	bool     m_needRemove;
	String   m_opponent; // ��ǰ���߶�ս�ĵз��û���
};