#include "stdafx.h"
#include "CServer.h"
#include "CUserMenager.h"

#include <thread>

CUserManager::CUserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer) :
	m_connectSocket(connectSocket), m_clientAddr(clientAddr), m_hServer(hServer),
	m_clientRecvThread(nullptr), m_clientSendThread(nullptr)
{
	m_clientRecvThread = new std::thread{ std::bind(&CUserManager::_client_recv_thread_, this) };
	m_clientRecvThread->detach();

	m_clientSendThread = new std::thread{ std::bind(&CUserManager::_client_send_thread_, this) };
	m_clientSendThread->detach();

	m_sendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_sendClosedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_recvClosedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CUserManager::~CUserManager()
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

void CUserManager::WritePacket(const Packet& packet)
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
	Packet recvPacket;

	char szRecvBuf[BUFLEN];
	while (!m_isClosed)
	{
		// Recive message from client
		std::memset(szRecvBuf, 0x0, sizeof(szRecvBuf));
		iRecvBytes = recv(m_connectSocket, szRecvBuf, sizeof(szRecvBuf), 0);
		
		if (iRecvBytes > 0)
		{
			std::memset(&recvPacket, 0x0, sizeof(recvPacket));
			sscanf(szRecvBuf, "ID=%d\n", &recvPacket.type);
			switch (recvPacket.type)
			{
			case Packet::Type::LAUNCH_REQUEST:
			{				
				sscanf(szRecvBuf, "ID=%d\nNAME=%s\nPASSWORD=%s\n", &recvPacket.type,
					&recvPacket.data.user_info.name, &recvPacket.data.user_info.password);

				printf("\nRequest: NAME=%s PASSWORD=%s\n\n>", recvPacket.data.user_info.name, recvPacket.data.user_info.password);

				CServer::Message message;
				message.type = CServer::Message::Type::CHECK_USER;
				std::strncpy(message.data.user_info.name, recvPacket.data.user_info.name, std::strlen(recvPacket.data.user_info.name));
				std::strncpy(message.data.user_info.password, recvPacket.data.user_info.password, std::strlen(recvPacket.data.user_info.password));
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
			CServer::Message message;
			message.type = CServer::Message::Type::USER_CLOSED;
			message.id = m_clientAddr.sin_addr.S_un.S_addr;

			m_hServer->WriteMessage(message);

			m_isClosed = true;
			SetEvent(m_recvClosedEvent);
			return;
		}
		else
		{
			CServer::Message message;
			message.type = CServer::Message::Type::USER_CLOSED;
			message.id = m_clientAddr.sin_addr.S_un.S_addr;

			m_hServer->WriteMessage(message);

			m_isClosed = true;
			SetEvent(m_recvClosedEvent);
			return;
		}
	}

	SetEvent(m_recvClosedEvent);
}

void CUserManager::_client_send_thread_()
{
	Packet sendPacket;
	int iSendBytes = 0;

	char szSendBuf[BUFLEN];

	while (!m_isClosed)
	{
		WaitForSingleObject(m_sendEvent, 5000);
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

		if (sendValid && sendPacket.type != Packet::Type::INVALID)
		{
			std::memset(szSendBuf, 0x0, sizeof(szSendBuf));
			switch (sendPacket.type)
			{
			case Packet::Type::LAUNCH_SUCCESS:
			case Packet::Type::LAUNCH_FAILED:
				sprintf(szSendBuf, "ID=%d\n", sendPacket.type);
				break;

			default:
				break;
			}

			iSendBytes = send(m_connectSocket,
				szSendBuf,
				std::strlen(szSendBuf) + 1,
				0);

			if (iSendBytes == SOCKET_ERROR)
			{
				CServer::Message message;
				message.type = CServer::Message::Type::USER_CLOSED;
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