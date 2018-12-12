#pragma once

/// <summary>
/// �û����洢����
/// </summary>
constexpr int USER_NAME_LENGTH = 40;
typedef char Username[USER_NAME_LENGTH];

/// <summary>
/// �û�����洢����
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
/// ��������
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
	// ���ݿ�ʵ��
	HDatabase   m_hDatabase;

	// ͨ��ʵ��
	Socket      m_serverSocket;
	SockaddrIn  m_serverAddr;

	// �û�ʵ��
	OnlineUsers m_onlineUsers;
	Mutex m_onlineUserLocker;

	RankedUsers m_rankedUsers;
	Mutex m_rankedUserLocker;

	/* �ص�IOģ�� */
	Handle  m_completionPort;
	Thread  m_accepter;
	Threads m_workers;
	volatile Boolean m_errorOccured;
	volatile Boolean m_isServerOn;

	/* ���ͷŵ��ص�IO��Դ��ȷ���ص�IO��Դ�����ظ��ͷţ� */
	std::map<Socket, Boolean> m_needRelease;
	Mutex m_releaseLocker;

private:
	/* ��ʼ������ */
	bool _InitDatabase_();
	bool _InitNetwork_();
	void _LoadRankedUsers_();

	/* ���ݰ�������� */
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

	/* IO���� */
	bool _SendPacket_(HOnlineUser user, const Packet& send);
	bool _RecvPacket_(HOnlineUser onlineUser);

private:
	/* IOCP����ģ�� */
	void _WorkerThread_();
	void _ServerAcceptThread_();
};