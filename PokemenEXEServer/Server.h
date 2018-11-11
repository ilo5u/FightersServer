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
	Thread m_acceptDriver;
	Threads m_workers;
	volatile Boolean m_errorOccured;
	volatile Boolean m_isServerOn;

private:
	bool _InitDatabase_();
	bool _InitNetwork_();

	bool _AnalyzePacket_(SockaddrIn client, const Packet& recv);

	void _DealWithLogin_(ULONG identity, const char data[]);
	void _DealWithLogon_(ULONG identity, const char data[]);
	void _DealWithLogout_(ULONG identity);

	void _DealWithGetOnlineUsers_(ULONG identity, const char data[]);
	void _DealWithPVEResult_(ULONG identity, const char data[]);

	void _SendPacket_(HUser user, const Packet& send);

private:
	void _WorkerThread_();
	void _ServerAcceptThread_();

};