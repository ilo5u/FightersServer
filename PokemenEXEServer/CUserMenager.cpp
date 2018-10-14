#include "stdafx.h"
#include "CServer.h"
#include "CUserMenager.h"

#include <thread>

CUserManager::CUserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer) :
	m_connectSocket(connectSocket), m_clientAddr(clientAddr), m_hServer(hServer),
	m_clientRecvThread(nullptr), m_clientSendThread(nullptr)
{
	m_clientRecvThread = new std::thread{ std::bind(&CUserManager::_client_recv_thread_, this) };
	m_clientSendThread = new std::thread{ std::bind(&CUserManager::_client_send_thread_, this) };

	m_sendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CUserManager::~CUserManager()
{
	if (m_clientSendThread->joinable())
	{
		m_clientSendThread->join();
	}
	delete m_clientSendThread;
	m_clientSendThread = nullptr;

	if (m_clientRecvThread->joinable())
	{
		m_clientRecvThread->join();
	}
	delete m_clientRecvThread;
	m_clientRecvThread = nullptr;
}

void CUserManager::WritePacket(const CUserManager::Packet& packet)
{
	m_sendMutex.lock();

	m_sendPacketQueue.push(packet);
	SetEvent(m_sendEvent);

	m_sendMutex.unlock();
}

int CUserManager::GetID() const
{
	return m_clientAddr.sin_addr.S_un.S_addr;
}

constexpr int DEFAULT_BUFFLEN = 4096;

void CUserManager::_client_recv_thread_()
{
	int iRecvBytes = 0;
	int iRecvBufLen = sizeof(Packet);
	Packet recvPacket;
	while (true)
	{
		// Recive message from client
		iRecvBufLen = DEFAULT_BUFFLEN;
		iRecvBytes = recv(m_connectSocket, (char*)&recvPacket, iRecvBufLen, 0);
		if (iRecvBytes > 0)
		{
			switch (recvPacket.type)
			{
			case Packet::Type::LAUNCH_REQUEST:
			{
				CServer::Message message;
				message.type = CServer::Message::Type::CHECK_USER;
				std::strncpy(message.data.user_info.name, recvPacket.data.user_info.name, NAME_LENGTH);
				std::strncpy(message.data.user_info.password, recvPacket.data.user_info.password, PASSWORD_LENGTH);
				message.id = m_clientAddr.sin_addr.S_un.S_addr;

				m_hServer->WriteMessage(message);
			}
			break;

			default:
				break;
			}
		}
		else if (iRecvBytes == 0)
		{	// close
			return;
		}
		else
		{
			closesocket(m_connectSocket);
			return;
		}
	}
}

void CUserManager::_client_send_thread_()
{
	Packet sendPacket;
	int iSendBytes = 0;
	int iSendTotal = 0;
	while (true)
	{
		WaitForSingleObject(m_sendEvent, 1000);
		ResetEvent(m_sendEvent);

		bool sendValid = false;
		m_sendMutex.lock();

		if (!m_sendPacketQueue.empty())
		{
			sendValid = true;
			sendPacket = m_sendPacketQueue.front();
			m_sendPacketQueue.pop();
		}

		m_sendMutex.unlock();

		if (sendValid)
		{
			while (iSendTotal < sizeof(Packet))
			{
				iSendBytes = send(m_connectSocket,
					(char*)&sendPacket + iSendTotal,
					sizeof(Packet) - iSendTotal,
					0);
				if (iSendBytes == SOCKET_ERROR)
				{
					closesocket(m_connectSocket);
					return;
				}
				else
				{
					iSendTotal += iSendBytes;
				}
			}

			iSendBytes = 0;
			iSendTotal = 0;
		}
	}
}

CUserManager::Packet::Packet()
{
}

CUserManager::Packet::Packet(const Packet& other) :
	type(other.type)
{
	switch (type)
	{
	case Type::LAUNCH_REQUEST:
	case Type::LAUNCH_SUCCESS:
	case Type::LAUNCH_FAILED:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		break;

	default:
		break;
	}
}

CUserManager::Packet::Packet(Packet&& other) :
	type(other.type)
{
	switch (type)
	{
	case Type::LAUNCH_REQUEST:
	case Type::LAUNCH_SUCCESS:
	case Type::LAUNCH_FAILED:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		break;

	default:
		break;
	}
}

CUserManager::Packet& CUserManager::Packet::operator=(const Packet & other)
{
	type = other.type;
	switch (type)
	{
	case Type::LAUNCH_REQUEST:
	case Type::LAUNCH_SUCCESS:
	case Type::LAUNCH_FAILED:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		break;

	default:
		break;
	}
	return *this;
}

CUserManager::Packet& CUserManager::Packet::operator=(Packet && other)
{
	type = other.type;
	switch (type)
	{
	case Type::LAUNCH_REQUEST:
	case Type::LAUNCH_SUCCESS:
	case Type::LAUNCH_FAILED:
		std::strncpy(data.user_info.name, other.data.user_info.name, NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		break;

	default:
		break;
	}
	return *this;
}
