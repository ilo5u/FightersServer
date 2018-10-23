#pragma once

typedef std::thread * HTHREAD;

class UserManager
{
public:
	enum class BattleType {
		PVE_LEVELUP, PVE_PK, PVP
	};

public:
	UserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer);
	~UserManager();

public:
	void InsertAPokemen(const std::string& info);

public:
	void WritePacket(const Packet& packet);
	ULONG GetID() const;
	void SetName(const std::string& name);
	std::string GetName() const;

private:
	const SOCKET m_connectSocket;
	const SOCKADDR_IN m_clientAddr;
	const HSERVER m_hServer;
	std::string m_userName;

	HTHREAD m_readBattleMessageThread;
	HTHREAD m_clientRecvThread;
	HTHREAD m_clientSendThread;

	MUTEX m_sendMutex;
	HANDLE m_sendEvent;
	std::queue<Packet> m_sendPacketQueue;

	bool m_isClosed = false;
	HANDLE m_recvClosedEvent;
	HANDLE m_sendClosedEvent;

private:
	struct Property
	{
		Pokemen::PokemenType type;
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
	BattleType m_battleType;
	bool m_isReadBattleMessageClosed;
	Pokemen::BattleStage m_battle_stage;
	std::list<Pokemen::PokemenManager> m_pokemens;

private:

	void _read_battle_message_();
	void _client_recv_thread_();
	void _client_send_thread_();
};
typedef UserManager * HUSERMANAGER;