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

private:
	HDATABASE m_hDatabase;

	SOCKET m_serverSocket;
	SOCKADDR_IN m_serverAddr;

	MUTEX m_userListMutex;
	USER_LIST m_userList;

private:
	int _init_network_();

	void _accept_();
};
typedef CServer * HSERVER;