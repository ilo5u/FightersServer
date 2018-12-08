#pragma once

constexpr int USER_NAME_LENGTH = 40;
typedef char Username[USER_NAME_LENGTH];

constexpr int PASSWORD_LENGTH = 80;
typedef char UserPassword[PASSWORD_LENGTH];

typedef Database    * HDatabase;
typedef User        * HUser;
typedef OnlineUser  * HOnlineUser;
typedef std::string   String;
typedef std::mutex    Mutex;
typedef SOCKET        Socket;
typedef SOCKADDR_IN   SockaddrIn;
typedef HANDLE        Handle;

typedef std::vector<String> Strings;
typedef std::vector<Thread> Threads;
typedef std::map<ULONG, HOnlineUser> OnlineUsers;
typedef std::list<HUser> RankedUsers;

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
	OnlineUsers m_onlineUsers;
	Mutex m_onlineUserLocker;

	RankedUsers m_rankedUsers;
	Mutex m_rankedUserLocker;

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
	void _LoadAllUsers_();

	bool _AnalyzePacket_(LPPER_HANDLE_DATA client, const Packet& recv);

	void _DealWithLogin_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithLogon_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithLogout_(LPPER_HANDLE_DATA client);

	void _DealWithGetOnlineUsers_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPVEResult_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPromotePokemen_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithAddPokemen_(LPPER_HANDLE_DATA client);
	void _DealWithSubPokemen_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithGetPokemensByUser_(LPPER_HANDLE_DATA client, const char data[]);

	void _OnLoginSuccessCallBack(HOnlineUser onlineUser, const Strings& userInfos, const Strings& queryElems);
	void _OnConnectionLostCallBack_(LPPER_HANDLE_DATA lostClient, LPPER_IO_OPERATION_DATA lostIO);
	void _OnUpdatePokemensCallBack_(HOnlineUser onlineUser);
	void _OnRenewRanklistCallBack_(HOnlineUser onlineUser);


	bool _SendPacket_(HOnlineUser user, const Packet& send);

private:
	void _WorkerThread_();
	void _ServerAcceptThread_();
	//void _BeatThread_();

};