#include "stdafx.h"

#include <thread>

UserManager::UserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer) :
	m_connectSocket(connectSocket), m_clientAddr(clientAddr), m_hServer(hServer),
	m_clientRecvThread(nullptr), m_clientSendThread(nullptr)
{
	m_clientRecvThread = new std::thread{ std::bind(&UserManager::_client_recv_thread_, this) };
	m_clientRecvThread->detach();

	m_clientSendThread = new std::thread{ std::bind(&UserManager::_client_send_thread_, this) };
	m_clientSendThread->detach();

	m_sendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_sendClosedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_recvClosedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

UserManager::~UserManager()
{
	WaitForSingleObject(m_recvClosedEvent, INFINITE);
	WaitForSingleObject(m_sendClosedEvent, INFINITE);

	CloseHandle(m_recvClosedEvent);
	CloseHandle(m_sendClosedEvent);
	CloseHandle(m_sendEvent);

	/*
	if (m_clientSendThread->joinable())
	{
		m_clientSendThread->join();
	}
	*/
	delete m_clientSendThread;
	m_clientSendThread = nullptr;

	/*
	if (m_clientRecvThread->joinable())
	{
		m_clientRecvThread->join();
	}
	*/
	delete m_clientRecvThread;
	m_clientRecvThread = nullptr;

	closesocket(m_connectSocket);
}

void UserManager::WritePacket(const Packet& packet)
{
	m_sendMutex.lock();

	m_sendPacketQueue.push(packet);
	SetEvent(m_sendEvent);

	m_sendMutex.unlock();
}

ULONG UserManager::GetID() const
{
	return m_clientAddr.sin_addr.S_un.S_addr;
}

void UserManager::SetName(const std::string& name)
{
	if (m_userName.empty())
		m_userName = name;
}

std::string UserManager::GetName() const
{
	return m_userName;
}

constexpr int DEFAULT_BUFFLEN = 4096;

void UserManager::_client_recv_thread_()
{
	int iRecvBytes = 0;
	Packet::Type type;

	char szRecvBuf[BUFLEN];
	Server::Message message;
	while (!m_isClosed)
	{
		// Recive message from client
		std::memset(szRecvBuf, 0x0, sizeof(szRecvBuf));
		iRecvBytes = recv(m_connectSocket, szRecvBuf, sizeof(szRecvBuf), 0);
		
		message.id = m_clientAddr.sin_addr.S_un.S_addr;
		if (iRecvBytes > 0)
		{
			sscanf(szRecvBuf, "ID=%d\n", &type);
			switch (type)
			{
			case Packet::Type::LAUNCH_REQUEST:
			{
				if (m_userName.empty())
				{
					message.type = Server::Message::Type::USER_LAUNCH;
					sscanf(szRecvBuf, "ID=%d\nNAME=%s\nPASSWORD=%s\n", &type,
						message.data.user_info.name, message.data.user_info.password);

					printf("\nµÇÂ½ÇëÇó£º NAME=%s PASSWORD=%s\n\n",
						message.data.user_info.name, message.data.user_info.password);

					m_hServer->WriteMessage(message);
				}
			}
			break;

			case Packet::Type::REGISTER_REQUEST:
			{
				message.type = Server::Message::Type::USER_REGISTER;
				sscanf(szRecvBuf, "ID=%d\nNAME=%s\nPASSWORD=%s\n", &type,
					message.data.user_info.name, message.data.user_info.password);

				printf("\n×¢²áÇëÇó£º NAME=%s PASSWORD=%s\n\n",
					message.data.user_info.name, message.data.user_info.password);

				m_hServer->WriteMessage(message);
			}
			break;

			case Packet::Type::GET_ONLINE_USERS:
			{
				message.type = Server::Message::Type::GET_ONLINE_USERS;
				m_hServer->WriteMessage(message);
			}
			break;

			case Packet::Type::BATTLE_REQUEST:
			{

			}
			break;

			case Packet::Type::DISCONNECT:
			{
				message.type = Server::Message::Type::USER_CLOSED;

				char szHostName[BUFSIZ];
				sscanf(szRecvBuf, "ID=%d\nHOST=%s\n", &type, szHostName);
				printf("\nµÇ³öÇëÇó£º HOST=%s\n\n>", szHostName);

				m_hServer->WriteMessage(message);

				m_isClosed = true;
				SetEvent(m_recvClosedEvent);
				return;
			}
			break;

			default:
				break;
			}
		}
		else
		{	// close
			message.type = Server::Message::Type::USER_CLOSED;

			m_hServer->WriteMessage(message);

			m_isClosed = true;
			SetEvent(m_recvClosedEvent);
			return;
		}
	}

	SetEvent(m_recvClosedEvent);
}

void UserManager::_client_send_thread_()
{
	Packet sendPacket;
	int iSendBytes = 0;

	char szSendBuf[BUFLEN];

	while (!m_isClosed)
	{
		WaitForSingleObject(m_sendEvent, 2000);

		bool sendValid = false;
		m_sendMutex.lock();
		ResetEvent(m_sendEvent);

		if (!m_sendPacketQueue.empty())
		{
			sendValid = true;
			sendPacket = m_sendPacketQueue.front();
			m_sendPacketQueue.pop();
		}

		m_sendMutex.unlock();

		if (sendValid && sendPacket.type != Packet::Type::INVALID)
		{
			std::memset(szSendBuf, 0x0, sizeof(szSendBuf));
			switch (sendPacket.type)
			{
			case Packet::Type::LAUNCH_SUCCESS:
			case Packet::Type::LAUNCH_FAILED:
			case Packet::Type::REGISTER_SUCCESS:
			case Packet::Type::REGISTER_FAILED:
			case Packet::Type::DISTRIBUTE_POKEMENS:
			{
				sprintf(szSendBuf, "ID=%u\n", sendPacket.type);
			}
			break;

			case Packet::Type::INSERT_A_POKEMEN:
			case Packet::Type::UPDATE_USERS:
			case Packet::Type::SET_ONLINE_USERS:
			{
				sprintf(szSendBuf, "ID=%u\n", sendPacket.type);
				int iLen = (int)std::strlen(szSendBuf);
				std::memcpy(szSendBuf + iLen, sendPacket.data, std::strlen(sendPacket.data));
			}
			break;

			default:
				break;
			}

			iSendBytes = send(m_connectSocket,
				szSendBuf,
				(int)std::strlen(szSendBuf) + 1,
				0);

			if (iSendBytes == SOCKET_ERROR)
			{
				Server::Message message;
				message.type = Server::Message::Type::USER_CLOSED;
				message.id = m_clientAddr.sin_addr.S_un.S_addr;

				m_hServer->WriteMessage(message);

				m_isClosed = true;
				SetEvent(m_sendClosedEvent);
				return;
			}
		}
	}

	SetEvent(m_sendClosedEvent);
}