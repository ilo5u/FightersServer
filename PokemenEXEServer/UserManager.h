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

typedef std::list<Pokemen::PokemenManager> Pokemens;

class Server;
class UserManager
{
public:
	enum class BattleType
	{
		PVE_LEVELUP, PVE_PK, PVP
	};

public:
	UserManager(const Socket& connectSocket, const SockaddrIn& clientAddr, Server& server);
	~UserManager();

public:
	void   InsertAPokemen(const String& info);

public:
	void   WritePacket(const Packet& packet);
	ULONG  GetUserID() const;
	void   SetUsername(const String& username);
	String GetUsername() const;

private:
	// net communication devices
	const Socket      m_connectSocket;
	const SockaddrIn  m_clientAddr;
	      Mutex       m_sendMutex;
	      Handle      m_sendEvent;
		  Packets     m_sendPackets;

		  Boolean     m_connectionClosed;
		  Handle      m_recvClosedEvent;
		  Handle      m_sendClosedEvent;

	// database bound
	      Server&     m_server;

	// battle control devices
		  BattleType  m_battleType;
		  Thread      m_readBattleMessageThread;	  // start up when user requests for a battle
		  Boolean     m_isReadBattleMessageClosed;
          BattleStage m_battleStage;

    // unique performance
		  String      m_username;
		  Pokemens    m_pokemens;
		  
private:
	struct Property
	{
		PokemenType type;
		int hpoints;
		int attack;
		int defense;
		int agility;
		int brea;
		int critical;
		int hitratio;
		int parryratio;
		int exp;
		int level;
	};

private:
	void _ReadBattleMessageThread_();
	void _ClientRecvThread_();
	void _ClientSendThread_();
};