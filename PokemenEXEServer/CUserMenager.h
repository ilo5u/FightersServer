#pragma once

typedef std::thread * HTHREAD;

class CUserManager
{
public:
	struct Packet
	{
		enum class Type
		{
			LAUNCH_REQUEST,
			LAUNCH_SUCCESS,
			LAUNCH_FAILED
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

		Packet();
		Packet(const Packet& other);
		Packet(Packet&& other);
		Packet& operator=(const Packet& other);
		Packet& operator=(Packet&& other);
	};

public:
	CUserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer);
	~CUserManager();

public:
	void WritePacket(const CUserManager::Packet& packet);
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
private:
	void _client_recv_thread_();
	void _client_send_thread_();
};
typedef CUserManager * HUSERMANAGER;