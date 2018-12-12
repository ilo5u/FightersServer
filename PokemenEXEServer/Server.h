#pragma once

/// <summary>
/// 用户名存储长度
/// </summary>
constexpr int USER_NAME_LENGTH = 40;
typedef char Username[USER_NAME_LENGTH];

/// <summary>
/// 用户密码存储长度
/// </summary>
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

/// <summary>
/// 服务器类
/// </summary>
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
	int    Init();
	bool   Run();
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
	Handle  m_completionPort;
	Thread  m_accepter;
	Threads m_workers;
	volatile Boolean m_errorOccured;
	volatile Boolean m_isServerOn;

	/* 待释放的重叠IO资源（确保重叠IO资源不被重复释放） */
	std::map<Socket, Boolean> m_needRelease;
	Mutex m_releaseLocker;

private:
	/* 初始化服务 */
	bool _InitDatabase_();
	bool _InitNetwork_();
	void _LoadRankedUsers_();

	/* 数据包处理服务 */
	bool _AnalyzePacket_(LPPER_HANDLE_DATA client, const Packet& recv);

	void _DealWithLogin_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithLogon_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithLogout_(LPPER_HANDLE_DATA client);

	void _DealWithUpdatePokemens_(LPPER_HANDLE_DATA client);
	void _DealWithGetOnlineUsers_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPVEResult_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPromotePokemen_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithAddPokemen_(LPPER_HANDLE_DATA client);
	void _DealWithSubPokemen_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithGetPokemensByUser_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithUpdateRanklist_(LPPER_HANDLE_DATA client);

	void _DealWithPVPRequest_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPVPCancel_(LPPER_HANDLE_DATA client);
	void _DealWithPVPAccept_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPVPBusy_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPVPBattle_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPVPMessage_(LPPER_HANDLE_DATA client, const char data[]);
	void _DealWithPVPResult_(LPPER_HANDLE_DATA client, const char data[]);

	void _OnLoginSuccessCallBack(HOnlineUser onlineUser, const Strings& userInfos, const Strings& queryElems);
	void _OnConnectionLostCallBack_(LPPER_HANDLE_DATA lostClient, LPPER_IO_OPERATION_DATA lostIO);
	void _OnUpdatePokemensCallBack_(HOnlineUser onlineUser);
	void _OnRenewRanklistCallBack_(HOnlineUser onlineUser);
	void _OnUpdateRanklistCallBack_(HOnlineUser onlineUser);
	void _OnRemovePokemenCallBack_(HOnlineUser onlineUser, int removeId);

	/* IO服务 */
	bool _SendPacket_(HOnlineUser user, const Packet& send);
	bool _RecvPacket_(HOnlineUser onlineUser);

private:
	/* IOCP工作模块 */
	void _WorkerThread_();
	void _ServerAcceptThread_();
};