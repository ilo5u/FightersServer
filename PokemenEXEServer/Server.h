#pragma once

constexpr int USER_NAME_LENGTH = 40;
typedef char Username[USER_NAME_LENGTH];

constexpr int PASSWORD_LENGTH = 80;
typedef char UserPassword[PASSWORD_LENGTH];

typedef Database    * HDatabase;
typedef User * HUser;
typedef std::string   String;
typedef std::mutex    Mutex;
typedef SOCKET        Socket;
typedef SOCKADDR_IN   SockaddrIn;
typedef HANDLE        Handle;

typedef std::vector<String>       Strings;
typedef std::vector<Thread> Threads;
typedef typename std::list<HUser> UserList;
typedef User::BattleType   BattleType;
typedef std::map<ULONG, HUser> Users;

class Server
{
public:
	Server();
	~Server();

public:
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

public:
	int  Init();
	bool Run();
	String GetClients() const;

private:
	// 数据库实例
	HDatabase   m_hDatabase;

	// 通信实例
	Socket      m_serverSocket;
	SockaddrIn  m_serverAddr;

	// 用户实例
	Users m_users;
	Mutex m_userLocker;

	/* 重叠IO模块 */
	Handle m_completionPort;
	Thread m_accepter;
	Threads m_workers;
	Thread m_beater;
	volatile Boolean m_errorOccured;
	volatile Boolean m_isServerOn;

	std::map<Socket, Boolean> m_needRelease;
	Mutex m_releaseLocker;

private:
	bool _InitDatabase_();
	bool _InitNetwork_();

	bool _AnalyzePacket_(SockaddrIn client, const Packet& recv, LPPER_IO_OPERATION_DATA perIO);

	void _DealWithLogin_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO);
	void _DealWithLogon_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO);
	void _DealWithLogout_(ULONG identity, LPPER_IO_OPERATION_DATA perIO);

	void _DealWithGetOnlineUsers_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO);
	void _DealWithPVEResult_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO);
	void _DealWithUpgradePokemen_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO);

	void _SendPacket_(HUser user, const Packet& send, LPPER_IO_OPERATION_DATA perIO);

private:
	void _WorkerThread_();
	void _ServerAcceptThread_();
	void _BeatThread_();

};