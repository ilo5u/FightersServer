#pragma once

typedef std::thread * HTHREAD;

class UserManager
{
public:

public:
	UserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer);
	~UserManager();

public:
	void WritePacket(const Packet& packet);
	ULONG GetID() const;
	void SetName(const std::string& name);
	std::string GetName() const;

private:
	const SOCKET m_connectSocket;
	const SOCKADDR_IN m_clientAddr;
	const HSERVER m_hServer;
	std::string m_userName;

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
typedef UserManager * HUSERMANAGER;