#include "stdafx.h"
#include "Server.h"
#include "User.h"

#include <thread>

typedef Packet::Type    PacketType;

OnlineUser::OnlineUser(const Socket& client, const SockaddrIn& clientAddr) :
	m_client(client), m_clientAddr(clientAddr),
	m_ioLocker(),
	m_ioHandlerCount(0x01),
	m_pokemens(), m_needRemove(false)
{
}

OnlineUser::~OnlineUser()
{
}

static Strings SplitData(const char data[])
{
	String per;
	Strings ans;
	for (int pos = 0; pos < std::strlen(data); ++pos)
	{
		if (data[pos] == '\n')
		{
			if (per.size() > 0)
				ans.push_back(per);
			per.clear();
		}
		else
		{
			per.push_back(data[pos]);
		}
	}
	if (per.size() > 0)
		ans.push_back(per);
	return ans;
}

// 仅由服务器线程调用
void OnlineUser::InsertAPokemen(const String& info)
{
	Strings pokemenInfos = SplitData(info.c_str());

	// 获取小精灵所有信息
	Pokemen::Pokemen pokemen{
		{
			std::atoi(pokemenInfos[1].c_str()),
			pokemenInfos[2].c_str(),
			std::atoi(pokemenInfos[3].c_str()),
			std::atoi(pokemenInfos[4].c_str()),
			std::atoi(pokemenInfos[5].c_str()),
			std::atoi(pokemenInfos[6].c_str()),
			std::atoi(pokemenInfos[7].c_str()),
			std::atoi(pokemenInfos[8].c_str()),
			std::atoi(pokemenInfos[9].c_str()),
			std::atoi(pokemenInfos[10].c_str()),
			std::atoi(pokemenInfos[12].c_str()),
			std::atoi(pokemenInfos[13].c_str()),
			std::atoi(pokemenInfos[0].c_str())
		},
		std::atoi(pokemenInfos[11].c_str())
	};
	m_pokemens.push_back(std::move(pokemen));
}


ULONG OnlineUser::GetUserID() const
{
	return m_clientAddr.sin_addr.S_un.S_addr;
}

void OnlineUser::SetUsername(const String& name)
{
	username = name;
}

std::string OnlineUser::GetUsername() const
{
	return username;
}

int OnlineUser::ReadIOCounter()
{
	this->m_ioLocker.lock();
	int count = this->m_ioHandlerCount;
	this->m_ioLocker.unlock();
	return count;
}

void OnlineUser::IncIOCounter()
{
	this->m_ioLocker.lock();
	this->m_ioHandlerCount++;
	this->m_ioLocker.unlock();
}

void OnlineUser::DecIOCounter()
{
	this->m_ioLocker.lock();
	this->m_ioHandlerCount--;
	this->m_ioLocker.unlock();
}
