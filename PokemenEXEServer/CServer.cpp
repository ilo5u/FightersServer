#include "stdafx.h"
#include "CServer.h"

#include "CDatabase.h"
#include "CUserMenager.h"

#include <thread>

constexpr int PORT = 27893;

constexpr int INIT_SUCCESS = 0x00000000;
constexpr int INIT_DATABASE_ERROR = 0xFFFFFFFE;
constexpr int INIT_NETWORK_ERROR = 0xFFFFFFFF;

constexpr auto DATABASE_USER = "root";
constexpr auto DATABASE_PASSWORD = "19981031";
constexpr auto DATABASE_NAME = "pokemen_user_database";

CServer::CServer() :
	m_hDatabase{new CDatabase{}},
	m_userList{}
{
}

CServer::~CServer()
{
	delete m_hDatabase;
}

int CServer::Init()
{
	if (m_hDatabase->Connect(DATABASE_USER, DATABASE_PASSWORD, DATABASE_NAME))
		return INIT_DATABASE_ERROR;

	if (_init_network_())
		return INIT_NETWORK_ERROR;

	return INIT_SUCCESS;
}

int CServer::Run()
{
	std::thread accept_thread{ std::bind(&CServer::_accept_, this) };
	accept_thread.detach();

	return INIT_SUCCESS;
}

int CServer::_init_network_()
{
	int iRetVal = 0;
	WSADATA wsaData;

	iRetVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRetVal != 0)
		return iRetVal;

	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_serverSocket == INVALID_SOCKET)
	{
		WSACleanup();
		return WSAGetLastError();
	}

	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.S_un.S_addr = inet_addr("10.201.6.236");
	m_serverAddr.sin_port = htons(PORT);

	iRetVal = bind(m_serverSocket, (LPSOCKADDR)&m_serverAddr,
		sizeof(SOCKADDR));
	if (iRetVal == SOCKET_ERROR)
	{
		closesocket(m_serverSocket);
		WSACleanup();
		return WSAGetLastError();
	}

	iRetVal = listen(m_serverSocket, 0);
	if (iRetVal == SOCKET_ERROR)
	{
		closesocket(m_serverSocket);
		WSACleanup();
		return WSAGetLastError();
	}

	return 0;
}

void CServer::_accept_()
{
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(SOCKADDR_IN);
	while (true)
	{
		SOCKET connectSocket = accept(m_serverSocket, (LPSOCKADDR)&clientAddr,
			&clientAddrLen);
		if (connectSocket == INVALID_SOCKET)
		{
			closesocket(m_serverSocket);
			WSACleanup();
			return;
		}

		m_userListMutex.lock();
		m_userList.push_back(new CUserManager{ connectSocket, clientAddr, this });
		m_userListMutex.unlock();
	}
}
