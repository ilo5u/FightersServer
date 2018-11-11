#include "stdafx.h"
#include "Server.h"
#include "User.h"

#include <thread>

typedef Packet::Type    PacketType;

User::User(const SockaddrIn& clientAddr) :
	m_client(), m_clientAddr(clientAddr),
	m_username(), m_pokemens()
{
}

User::~User()
{
}

static Strings SplitData(const char data[])
{
	String per;
	Strings ans;
	for (int pos = 0; pos < std::strlen(data); ++pos)
	{
		if (pos == '\n')
		{
			if (!per.empty())
				ans.push_back(per);
			per.clear();
		}
		else
		{
			per.push_back(data[pos]);
		}
	}
	return std::move(ans);
}

// 仅由服务器线程调用
void User::InsertAPokemen(const String& info)
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
			std::atoi(pokemenInfos[13].c_str())
		},
		std::atoi(pokemenInfos[11].c_str())
	};
	m_pokemens.push_back(std::move(pokemen));
}


ULONG User::GetUserID() const
{
	return m_clientAddr.sin_addr.S_un.S_addr;
}

void User::SetUsername(const String& name)
{
	m_username = name;
}

std::string User::GetUsername() const
{
	return m_username;
}