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

	m_recvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	std::thread recv_thread{ std::bind(&CServer::_server_recv_thread_, this) };
	recv_thread.detach();

	std::thread send_thread{ std::bind(&CServer::_server_send_thread_, this) };
	send_thread.detach();

	return INIT_SUCCESS;
}

void CServer::WriteMessage(const Message& message)
{
	m_recvMutex.lock();

	m_recvMessageQueue.push(message);
	SetEvent(m_recvEvent);

	m_recvMutex.unlock();
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

void CServer::_server_control_thread_()
{
}

void CServer::_server_recv_thread_()
{
	Message message;
	std::vector<std::string> queryResult;
	while (true)
	{
		WaitForSingleObject(m_recvEvent, 1000);
		ResetEvent(m_recvEvent);
		m_recvMutex.lock();

		bool recvValid = false;
		if (!m_recvMessageQueue.empty())
		{
			recvValid = true;
			message = m_recvMessageQueue.front();
			m_recvMessageQueue.pop();
		}

		m_recvMutex.unlock();

		if (recvValid)
		{
			switch (message.type)
			{
			case Message::Type::CHECK_USER:
			{
				std::string queryStr{ "select password from user_launch_info where name=\"" };
				queryStr += message.data.user_info.name + std::string{ "\""};
				queryResult = m_hDatabase->Select(queryStr, 1);

				USER_LIST::iterator it = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUSERMANAGER& hUserManager) {
					return hUserManager->GetID() == message.id;
				});
				CUserManager::Packet packet;
				if (it != m_userList.end())
				{	// found
					if (std::strcmp(queryResult[0].c_str(), message.data.user_info.password) == 0)
					{	// matched
						m_userListMutex.lock();

						packet.type = CUserManager::Packet::Type::LAUNCH_SUCCESS;
						(*it)->WritePacket(packet);

						m_userListMutex.unlock();
					}
					else
					{
						m_userListMutex.lock();

						packet.type = CUserManager::Packet::Type::LAUNCH_FAILED;
						(*it)->WritePacket(packet);

						m_userListMutex.unlock();
					}
				}

				queryResult.clear();
			}
			break;

			default:
				break;
			}
		}
	}
}

void CServer::_server_send_thread_()
{
	while (true)
	{

	}
}

CServer::Message::Message()
{
}

CServer::Message::Message(const Message& other) :
	type(other.type)
{
	switch (type)
	{
	case Type::CHECK_USER:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	default:
		break;
	}
}

CServer::Message::Message(Message&& other) :
	type(other.type)
{
	switch (type)
	{
	case Type::CHECK_USER:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	default:
		break;
	}
}

CServer::Message& CServer::Message::operator=(const Message& other)
{
	type = other.type;
	switch (type)
	{
	case Type::CHECK_USER:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	default:
		break;
	}
	return *this;
}

CServer::Message& CServer::Message::operator=(Message && other)
{
	type = other.type;
	switch (type)
	{
	case Type::CHECK_USER:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	default:
		break;
	}
	return *this;
}
