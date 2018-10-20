#pragma once

constexpr int USER_NAME_LENGTH = 40;
typedef char USER_NAME[USER_NAME_LENGTH];

constexpr int PASSWORD_LENGTH = 80;
typedef char USER_PASSWORD[PASSWORD_LENGTH];

class Database;
typedef Database * HDATABASE;

class UserManager;
typedef UserManager * HUSERMANAGER;

typedef typename std::list<HUSERMANAGER> USER_LIST;
typedef typename std::mutex MUTEX;

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

			GET_ONLINE_USERS
		};

		struct UserInfo
		{
			USER_NAME name;
			USER_PASSWORD password;
		};

		Type type;
		union Data
		{
			UserInfo user_info;
		};
		Data data;
		ULONG id;

		Message();
		Message(const Message& other);
		Message(Message&& other);
		Message& operator=(const Message& other);
		Message& operator=(Message&& other);
	};

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
	
	std::string GetClients() const;

private:
	HDATABASE m_hDatabase;

	SOCKET m_serverSocket;
	SOCKADDR_IN m_serverAddr;

	MUTEX m_userListMutex;
	USER_LIST m_userList;

	MUTEX m_recvMutex;
	HANDLE m_recvEvent;
	std::queue<Message> m_recvMessageQueue;

private:
	int _init_network_();
	void _deal_with_user_launch_(const Message& message);
	void _deal_with_user_register_(const Message& message);
	void _deal_with_user_closed_(const Message& message);
	void _deal_with_get_online_users_(const Message& message);

private:
	void _accept_();

	void _server_recv_thread_();
	void _server_send_thread_();
};
typedef Server * HSERVER;