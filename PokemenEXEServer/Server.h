#pragma once

constexpr int USER_NAME_LENGTH = 40;
typedef char Username[USER_NAME_LENGTH];

constexpr int PASSWORD_LENGTH = 80;
typedef char UserPassword[PASSWORD_LENGTH];

typedef Database    * HDatabase;
typedef UserManager * HUser;
typedef std::string   String;
typedef std::mutex    Mutex;
typedef SOCKET        Socket;
typedef SOCKADDR_IN   SockaddrIn;
typedef HANDLE        Handle;

typedef std::vector<String>       Strings;
typedef typename std::list<HUser> UserList;
typedef UserManager::BattleType   BattleType;

class Server
{
public:
	struct Message
	{
		enum class Type
		{
			USER_LAUNCH,
			USER_REGISTER,
			USER_CLOSED,

			GET_ONLINE_USERS,

			HANDLE_BATTLE_RESULT
		};

		struct User
		{
			Username     name;
			UserPassword password;
		};

		struct BattleResult
		{
			UserManager::BattleType battleType;
			int winner;
			int rounds;
			int firstId;
			int secondId;
		};

		struct PVP
		{
			int firstId;
			int secondId;
		};

		Type type;
		union Data
		{
			User         user;
			BattleResult battleResult;
			PVP          pvp;
		};
		Data  data;
		ULONG id;

		Message();
		Message(const Message& other);
		Message(Message&& other);
		Message& operator=(const Message& other);
		Message& operator=(Message&& other);
	};
	typedef std::queue<Message> Messages;
	typedef Message::Type       MessageType;

public:
	Server();
	~Server();

public:
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator=(const Server&) = delete;
	Server& operator=(Server&&) = delete;

public:
	int Init();
	int Run();
	void WriteMessage(const Message& message);
	String GetClients() const;

private:
	// ���ݿ�ʵ��
	HDatabase   m_hDatabase;

	// ͨ��ʵ��
	Socket      m_serverSocket;
	SockaddrIn  m_serverAddr;

	Mutex       m_recvMutex;
	Handle      m_recvEvent;
	Messages    m_recvMessages;

	// �û�ʵ��
	UserList    m_userList;
	Mutex       m_userListMutex;

private:
	int  _InitNetwork_();
	void _DealWithUserLaunch_(const Message& message);
	void _DealWithUserRegister_(const Message& message);
	void _DealWithUserClosed_(const Message& message);
	void _DealWithGetOnlineUsers_(const Message& message);
	void _DealWithBattleResult_(const Message& message);

private:
	void _ServerAcceptThread_();
	void _ServerRecvThread_();
	void _ServerSendThread_();
};