#pragma once

class CDatabase;
typedef CDatabase * HDATABASE;

class CUserManager;
typedef CUserManager * HUSERMANAGER;

typedef typename std::list<HUSERMANAGER> USER_LIST;
typedef typename std::mutex MUTEX;

class CServer
{
public:
	struct Message
	{
		enum class Type
		{
			CHECK_USER
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
		int id;

		Message();
		Message(const Message& other);
		Message(Message&& other);
		Message& operator=(const Message& other);
		Message& operator=(Message&& other);
	};

public:
	CServer();
	~CServer();

public:
	CServer(const CServer&) = delete;
	CServer(CServer&&) = delete;
	CServer& operator=(const CServer&) = delete;
	CServer& operator=(CServer&&) = delete;

public:
	int Init();
	int Run();

	void WriteMessage(const Message& message);

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

	void _accept_();

	void _server_control_thread_();

	void _server_recv_thread_();
	void _server_send_thread_();
};
typedef CServer * HSERVER;