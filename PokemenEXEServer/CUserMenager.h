#pragma once

class CUserManager
{
public:
	CUserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer);
	~CUserManager();

private:
	SOCKET m_connectSocket;
	SOCKADDR_IN m_clientAddr;
	HSERVER m_hServer;
};
typedef CUserManager * HUSERMANAGER;