#include "stdafx.h"
#include "CServer.h"
#include "CUserMenager.h"

CUserManager::CUserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer) :
	m_connectSocket(connectSocket), m_clientAddr(clientAddr), m_hServer(hServer)
{
}

CUserManager::~CUserManager()
{
}
