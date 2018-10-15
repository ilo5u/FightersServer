#pragma once

typedef std::thread * HTHREAD;

class CUserManager
{
public:

public:
	CUserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer);
	~CUserManager();

public:
	void WritePacket(const Packet& packet);
	int GetID() const;

private:
	SOCKET m_connectSocket;
	SOCKADDR_IN m_clientAddr;
	HSERVER m_hServer;

	HTHREAD m_clientRecvThread;
	HTHREAD m_clientSendThread;

	MUTEX m_sendMutex;
	HANDLE m_sendEvent;
	std::queue<Packet> m_sendPacketQueue;

	bool m_isClosed = false;
	HANDLE m_recvClosedEvent;
	HANDLE m_sendClosedEvent;
private:
	void _client_recv_thread_();
	void _client_send_thread_();
};
typedef CUserManager * HUSERMANAGER;