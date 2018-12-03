#include "stdafx.h"
#include "Database.h"

typedef Packet::Type PacketType;

constexpr int  PORT = 27893;

constexpr int  INIT_SUCCESS        = 0x00000000;
constexpr int  INIT_DATABASE_ERROR = 0xFFFFFFFE;
constexpr int  INIT_NETWORK_ERROR  = 0xFFFFFFFF;

constexpr auto DATABASE_USER     = "root";
constexpr auto DATABASE_PASSWORD = "19981031";
constexpr auto DATABASE_NAME     = "server";

#define NUMBER_OF_POKEMEN_COLUMNS 14

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

static Strings SplitData(const char data[], char ch)
{
	String per;
	Strings ans;
	for (int pos = 0; pos < std::strlen(data); ++pos)
	{
		if (data[pos] == ch)
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

Server::Server() :
	m_hDatabase{ new Database{} },
	m_serverSocket(), m_serverAddr(),
	m_users(), m_userLocker(),
	m_completionPort(nullptr), 
	m_accepter(), m_workers(), m_beater(),
	m_errorOccured(false),
	m_isServerOn(false)
{
}

Server::~Server()
{
	this->m_isServerOn = false;
	if (this->m_beater.joinable())
		this->m_beater.join();

	if (this->m_accepter.joinable())
		this->m_accepter.join();

	delete this->m_hDatabase;
}

int Server::Init()
{
	if (!_InitDatabase_())
		return INIT_DATABASE_ERROR;
	if (!_InitNetwork_())
		return INIT_NETWORK_ERROR;
	return INIT_SUCCESS;
}

bool Server::Run()
{
	if (this->m_isServerOn)
		return false;

	/* 创建服务线程 */
	try
	{
		// 创建新的完成端口
		if ((this->m_completionPort = CreateIoCompletionPort(
			INVALID_HANDLE_VALUE, NULL, 0, 0)
			) == NULL)
		{
			WSACleanup();
			throw std::exception("资源不足，无法创建完成端口。\n");
		}

		// 创建监听Socket
		if ((this->m_serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
			WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		{
			WSACleanup();
			throw std::exception("资源不足，无法创建Socket。\n");
		}

		m_serverAddr.sin_family = AF_INET;
		m_serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		m_serverAddr.sin_port = htons(PORT);

		if (bind(m_serverSocket, (LPSOCKADDR)&m_serverAddr,
			sizeof(SOCKADDR)) == SOCKET_ERROR)
		{
			closesocket(m_serverSocket);
			WSACleanup();
			throw std::exception("无法绑定到本地端口。\n");
		}

		if (listen(m_serverSocket, 0) == SOCKET_ERROR)
		{
			closesocket(m_serverSocket);
			WSACleanup();
			throw std::exception("监听端口失败。\n");
		}

		this->m_isServerOn = true;
		SYSTEM_INFO SystemInfo; // 系统信息
		GetSystemInfo(&SystemInfo);
		// 根据CPU数量启动线程
		for (int i = 0; i < (int)SystemInfo.dwNumberOfProcessors * 2; ++i)
		{ // 创建线程，运行ServerWorkerThread
			this->m_workers.push_back(
				std::move(Thread{ std::bind(&Server::_WorkerThread_, this) })
			);
		}
		this->m_accepter
			= std::move(Thread{ std::bind(&Server::_ServerAcceptThread_, this) });

		//this->m_beater
		//	= std::move(Thread{ std::bind(&Server::_BeatThread_, this) });
	}
	catch (std::exception&)
	{
		return false;
	}
	return true;
}

String Server::GetClients() const
{
	String queryResult;
	char   querySingle[BUFLEN];
	int    id = 0;
	for (const auto& user : this->m_users)
	{
		++id;
		SockaddrIn addr;
		addr.sin_addr.S_un.S_addr = user.first;
		sprintf(querySingle, "用户%d: IP=%s\n", id, inet_ntoa(addr.sin_addr));
		queryResult += querySingle;
	}

	if (queryResult.size() == 0)
	{
		queryResult = "无用户连接。\n";
	}
	return queryResult;
}

bool Server::_InitDatabase_()
{
	return m_hDatabase->Connect(DATABASE_USER, DATABASE_PASSWORD, DATABASE_NAME);
}

bool Server::_InitNetwork_()
{
	try
	{
		WSADATA wsaData;

		// 初始化Windows Socket环境
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			throw std::exception("初始化Windows Socket环境失败。\n");
		}
	}
	catch (const std::exception& e)
	{
		fprintf(stdout, "%s\n", e.what());
		return false;
	}
	return true;
}

bool Server::_AnalyzePacket_(SockaddrIn client, const Packet& recv, LPPER_IO_OPERATION_DATA perIO)
{
	switch (recv.type)
	{
	case PacketType::LOGIN_REQUEST:
		_DealWithLogin_(client.sin_addr.S_un.S_addr, recv.data, perIO);
		break;

	case PacketType::LOGON_REQUEST:
		_DealWithLogon_(client.sin_addr.S_un.S_addr, recv.data, perIO);
		break;

	case PacketType::GET_ONLINE_USERS:
		_DealWithGetOnlineUsers_(client.sin_addr.S_un.S_addr, recv.data, perIO);
		break;

	case PacketType::LOGOUT:
		_DealWithLogout_(client.sin_addr.S_un.S_addr, perIO);
		break;

	case PacketType::PVE_RESULT:
		_DealWithPVEResult_(client.sin_addr.S_un.S_addr, recv.data, perIO);
		break;

	case PacketType::UPGRADE_POKEMEN:
		_DealWithUpgradePokemen_(client.sin_addr.S_un.S_addr, recv.data, perIO);
		break;

	default:
		break;
	}
	return false;
}

#define INSERT_POKEMEN_QUERYSTRING 	"\
insert into pokemens(user,type,name,\
hpoints,attack,defense,agility,interva,critical,hitratio,parryratio,\
career,exp,level) values('%s',%d,'%s',\
%d,%d,%d,%d,%d,%d,%d,%d,\
%d,%d,%d)"

#define SELECT_POKEMEN_QUERYSTRING 	"\
select identity,type,name,\
hpoints,attack,defense,agility,interva,critical,hitratio,parryratio,\
career,exp,level from pokemens where user='%s'"

#define UPDATE_POKEMEN_QUERYSTRING "\
update pokemens set hpoints=%d,attack=%d,defense=%d,agility=%d,interva=%d,critical=%d,hitratio=%d,parryratio=%d,\
career=%d,exp=%d,level=%d where identity=%d"

#define POKEMEN_ALL_PROPERTIES "%d\n%d\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n"
const static int initNumberOfPokemens = 3;
void Server::_DealWithLogin_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO)
{
	Strings queryResult;
	char    szQuery[BUFLEN];

	Strings userInfos = SplitData(data);
	Packet  sendPacket;
	try
	{
		sprintf(szQuery, 
			"select numberOfPokemens,rounds,wins from users where name='%s' and password='%s'",
			userInfos[0].c_str(), userInfos[1].c_str());
		queryResult = m_hDatabase->Select(szQuery, 3);

		this->m_userLocker.lock();
		HUser user = this->m_users[identity];
		this->m_userLocker.unlock();
		if (user != nullptr)
		{
			fprintf(stdout,
				"收到登录请求：用户名=%s 密码=%s\n",
				userInfos[0].c_str(), userInfos[1].c_str());
			// 反馈登陆信息
			Strings queryElems;
			if (!user->m_username.empty() || queryResult.empty())
				sendPacket.type = PacketType::LOGIN_FAILED;
			else
			{
				queryElems = SplitData(queryResult[0].c_str());
				sendPacket.type = PacketType::LOGIN_SUCCESS;
				user->SetUsername(userInfos[0]);
				sprintf(sendPacket.data, "%s", queryElems[0].c_str());
				fprintf(stdout, "登陆成功\n");
			}
			_SendPacket_(user, sendPacket, perIO);

			// 反馈在线用户以及小精灵信息
			if (sendPacket.type == PacketType::LOGIN_SUCCESS)
			{
				user->m_username = userInfos[0];
				int numberOfPokemens = std::atoi(queryElems[0].c_str());
				user->m_rounds = std::atoi(queryElems[1].c_str());
				user->m_wins = std::atoi(queryElems[2].c_str());
				if (numberOfPokemens == 0)
				{
					for (int i = 0; i < initNumberOfPokemens; ++i)
					{
						/* 用户第一次登陆，随机生成三只小精灵 */
						Pokemen::Pokemen pokemen{ PokemenType::DEFAULT, 0x1 };
						sprintf(szQuery, INSERT_POKEMEN_QUERYSTRING,
							userInfos[0].c_str(),
							(int)pokemen.GetType(), pokemen.GetName().c_str(),
							pokemen.GetHpoints(), pokemen.GetAttack(),
							pokemen.GetDefense(), pokemen.GetAgility(),
							pokemen.GetInterval(), pokemen.GetCritical(),
							pokemen.GetHitratio(), pokemen.GetParryratio(),
							pokemen.GetCareer(),
							pokemen.GetExp(), pokemen.GetLevel()
						);
						this->m_hDatabase->Insert(szQuery);
					}
					sprintf(szQuery,
						"update users set numberOfPokemens=%d where name='%s'",
						initNumberOfPokemens, userInfos[0].c_str()
					);
					this->m_hDatabase->Update(szQuery);
				}

				sendPacket.type = PacketType::UPDATE_POKEMENS;
				/* 从数据库取出小精灵信息 */
				sprintf(szQuery, SELECT_POKEMEN_QUERYSTRING, userInfos[0].c_str());
				queryResult = this->m_hDatabase->Select(szQuery, NUMBER_OF_POKEMEN_COLUMNS);
				for (const auto& pokemen : queryResult)
				{
					user->InsertAPokemen(pokemen);
					sprintf(sendPacket.data, pokemen.c_str());
					_SendPacket_(user, sendPacket, perIO);
				}

				sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
				sprintf(sendPacket.data, "%s\nON\n", userInfos[0].c_str());
				this->m_userLocker.lock();
				for (const auto& other : this->m_users)
				{
					if (other.first != identity)
					{
						_SendPacket_(other.second, sendPacket, perIO);
					}
				}
				this->m_userLocker.unlock();
			}
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
	}
}

void Server::_DealWithLogon_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO)
{
	Strings queryResult;
	char    szQuery[BUFLEN];

	Strings userInfos = SplitData(data);
	Packet  sendPacket;
	try
	{
		this->m_userLocker.lock();
		HUser user = this->m_users[identity];
		this->m_userLocker.unlock();
		if (user != nullptr)
		{
			sprintf(szQuery,
				"insert into users values('%s','%s',0, 0, 0)",
				userInfos[0].c_str(), userInfos[1].c_str()
			);

			fprintf(stdout,
				"收到注册请求：用户名=%s 密码=%s\n",
				userInfos[0].c_str(), userInfos[1].c_str());

			if (this->m_hDatabase->Insert(szQuery))
				sendPacket.type = PacketType::LOGON_SUCCESS;
			else
				sendPacket.type = PacketType::LOGON_FAILED;
			_SendPacket_(user, sendPacket, perIO);
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
	}
}

void Server::_DealWithLogout_(ULONG identity, LPPER_IO_OPERATION_DATA perIO)
{
	Strings queryResult;
	Packet sendPacket;
	try
	{
		this->m_userLocker.lock();
		HUser user = this->m_users[identity];
		if (user != nullptr)
		{
			sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
			sprintf(sendPacket.data, "%s\nOFF\n", user->m_username.c_str());
			for (auto& other : this->m_users)
			{
				if (other.first != identity)
				{
					_SendPacket_(other.second, sendPacket, other.second->m_io);
				}
			}
			closesocket(user->m_client);
			this->m_users.erase(identity);
		}
		this->m_userLocker.unlock();
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
	}
}

void Server::_DealWithGetOnlineUsers_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO)
{
	Packet sendPacket;
	try
	{
		this->m_userLocker.lock();
		HUser user = this->m_users[identity];
		this->m_userLocker.unlock();
		if (user != nullptr)
		{
			char   szUserNames[BUFLEN];
			int    cnt = 0;
			sendPacket.type = PacketType::SET_ONLINE_USERS;

			ZeroMemory(szUserNames, sizeof(szUserNames));
			for (const auto& other : this->m_users)
			{
				if (other.first != identity)
				{
					++cnt;
					sprintf(szUserNames + std::strlen(szUserNames),
						"%s\n", other.second->GetUsername().c_str()
					);
					if (cnt == 0)
					{
						sprintf(sendPacket.data, "20\n%s", szUserNames);
						_SendPacket_(user, sendPacket, perIO);

						cnt = 0;
						ZeroMemory(szUserNames, sizeof(szUserNames));
					}
				}
			}
			if (cnt > 0)
			{
				sprintf(sendPacket.data, "%d\n%s", cnt, szUserNames);
				_SendPacket_(user, sendPacket, perIO);
			}
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
	}
}

void Server::_DealWithPVEResult_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO)
{
	Packet sendPacket;
	Strings infos = SplitData(data);
	try
	{
		this->m_userLocker.lock();
		HUser user = this->m_users[identity];
		this->m_userLocker.unlock();
		if (user != nullptr)
		{
			++user->m_rounds;
			if (infos[0].compare("F") == 0)
				++user->m_wins;

			fprintf(stdout,
				"比赛结果：%s", data);

			Strings queryResult;
			char    szQuery[BUFLEN];
			sprintf(szQuery,
				"update users set rounds=%d,wins=%d where name='%s'", 
				user->m_rounds, user->m_wins, user->m_username.c_str());
			this->m_hDatabase->Update(szQuery);

			Pokemen::Property prop{
				std::atoi(infos[4].c_str()), infos[5].c_str(),
				std::atoi(infos[6].c_str()), std::atoi(infos[7].c_str()), std::atoi(infos[8].c_str()), std::atoi(infos[9].c_str()),
				std::atoi(infos[10].c_str()), std::atoi(infos[11].c_str()), std::atoi(infos[12].c_str()), std::atoi(infos[13].c_str()),
				std::atoi(infos[15].c_str()), std::atoi(infos[16].c_str()),
				std::atoi(infos[3].c_str())
			};
			Pokemens::iterator pokemen = std::find_if(user->m_pokemens.begin(),
				user->m_pokemens.end(),
				[&prop](const Pokemen::Pokemen& temp) {
				return temp.GetId() == prop.m_id;
			});
			if (pokemen != user->m_pokemens.end())
			{
				pokemen->RenewProperty(prop, std::atoi(infos[14].c_str()));
				sprintf(szQuery,
					UPDATE_POKEMEN_QUERYSTRING,
					prop.m_hpoints, prop.m_attack, prop.m_defense, prop.m_agility,
					prop.m_interval, prop.m_critical, prop.m_hitratio, prop.m_parryratio,
					std::atoi(infos[14].c_str()), prop.m_exp, prop.m_level,
					prop.m_id
				);
				this->m_hDatabase->Update(szQuery);

				sendPacket.type = PacketType::UPDATE_POKEMENS;
				sprintf(sendPacket.data,
					POKEMEN_ALL_PROPERTIES,
					pokemen->GetId(), (int)pokemen->GetType(), pokemen->GetName().c_str(),
					pokemen->GetHpoints(), pokemen->GetAttack(), pokemen->GetDefense(), pokemen->GetAgility(),
					pokemen->GetInterval(), pokemen->GetCritical(), pokemen->GetHitratio(), pokemen->GetParryratio(),
					pokemen->GetCareer(), pokemen->GetExp(), pokemen->GetLevel()
				);
				_SendPacket_(user, sendPacket, perIO);
			}
		}
	}
	catch (const std::exception&)
	{
	}
}

void Server::_DealWithUpgradePokemen_(ULONG identity, const char data[], LPPER_IO_OPERATION_DATA perIO)
{
	Strings queryResult;
	char    szQuery[BUFLEN];
	Packet  sendPacket;
	try
	{ 
		this->m_userLocker.lock();
		HUser user = this->m_users[identity];
		this->m_userLocker.unlock();
		if (user != nullptr)
		{
			int pokemenId = 0;
			int careerType = 0;
			sscanf(data, "%d\n%d\n", &pokemenId, &careerType);
			Pokemens::iterator pokemen = std::find_if(user->m_pokemens.begin(),
				user->m_pokemens.end(),
				[&pokemenId](const Pokemen::Pokemen& temp){
					return temp.GetId() == pokemenId;
			});
			if (pokemen != user->m_pokemens.end() && careerType < 3)
			{
				pokemen->Promote(careerType);
				sprintf(szQuery,
					UPDATE_POKEMEN_QUERYSTRING,
					pokemen->GetHpoints(), pokemen->GetAttack(), pokemen->GetDefense(), pokemen->GetAgility(),
					pokemen->GetInterval(), pokemen->GetCritical(), pokemen->GetHitratio(), pokemen->GetParryratio(),
					pokemen->GetCareer(), pokemen->GetExp(), pokemen->GetLevel(),
					pokemenId
				);
				this->m_hDatabase->Update(szQuery);

				sendPacket.type = PacketType::UPDATE_POKEMENS;
				sprintf(sendPacket.data,
					POKEMEN_ALL_PROPERTIES,
					pokemen->GetId(), (int)pokemen->GetType(), pokemen->GetName().c_str(),
					pokemen->GetHpoints(), pokemen->GetAttack(), pokemen->GetDefense(), pokemen->GetAgility(),
					pokemen->GetInterval(), pokemen->GetCritical(), pokemen->GetHitratio(), pokemen->GetParryratio(),
					pokemen->GetCareer(), pokemen->GetExp(), pokemen->GetLevel()
				);
				_SendPacket_(user, sendPacket, perIO);
			}
		}
	}
	catch (const std::exception&)
	{
	}
}

/* 投递出境数据包 */
void Server::_SendPacket_(HUser user, const Packet& sendPacket, LPPER_IO_OPERATION_DATA perIO)
{
	try
	{
		/* 设置发送对象 */
		DWORD sendBytes;
		ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
		std::memcpy((LPCH)perIO->buffer, (LPCH)&sendPacket, sizeof(Packet));
		perIO->dataBuf.buf = perIO->buffer;
		perIO->dataBuf.len = sizeof(Packet);
		perIO->opType = OPERATION_TYPE::SEND_POSTED;
		perIO->sendBytes = 0;
		perIO->totalBytes = sizeof(Packet);

		if (WSASend(user->m_client, &(perIO->dataBuf), 1, &sendBytes, 0,
			&(perIO->overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
				throw std::exception("网络异常。\n");
		}
	}
	catch (const std::exception& e)
	{
		fprintf(stdout, "%s异常码：%d\n", e.what(), WSAGetLastError());
	}
}

void Server::_WorkerThread_()
{
	DWORD bytesTransferred; // 数据传输的字节数
	LPPER_HANDLE_DATA perClient; // Socket句柄结构体
	LPPER_IO_OPERATION_DATA perIO; // I/O操作结构体
	DWORD flags;
	DWORD recvBytes;
	DWORD sendBytes;

	Packet recvPacket;
	Packet sendPacket;

	while (!this->m_errorOccured && this->m_isServerOn)
	{
		perClient = nullptr;
		// 检查完成端口的状态
		if (GetQueuedCompletionStatus(this->m_completionPort, &bytesTransferred,
			(PULONG_PTR)&perClient, (LPOVERLAPPED*)&perIO, INFINITE) == 0)
		{
			if (perClient != nullptr)
			{
				this->m_userLocker.lock();
				this->m_releaseLocker.lock();
				if (this->m_needRelease[perClient->client])
				{
					HUser user = this->m_users[perClient->addr.sin_addr.S_un.S_addr];
					if (user != nullptr)
					{
						sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
						sprintf(sendPacket.data, "%s\nOFF\n", user->GetUsername().c_str());
						for (auto& other : this->m_users)
						{
							if (other.first != perClient->addr.sin_addr.S_un.S_addr)
							{
								_SendPacket_(other.second, sendPacket, other.second->m_io);
							}
						}
					}
					/* 释放连接 */
					this->m_users.erase(perClient->addr.sin_addr.S_un.S_addr);

					closesocket(perClient->client);
					this->m_needRelease[perClient->client] = false;
					this->m_needRelease.erase(perClient->client);
					delete perClient;
					delete perIO;
				}
				this->m_releaseLocker.unlock();
				this->m_userLocker.unlock();
				continue;
			}
		}
		else
		{
			// 如果数据传送完了，则退出（close）
			if (bytesTransferred == 0
				&& (perIO->opType == OPERATION_TYPE::RECV_POSTED
					|| perIO->opType == OPERATION_TYPE::SEND_POSTED))
			{
				this->m_userLocker.lock();
				this->m_releaseLocker.lock();
				if (this->m_needRelease[perClient->client])
				{
					HUser user = this->m_users[perClient->addr.sin_addr.S_un.S_addr];
					if (user != nullptr)
					{
						sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
						sprintf(sendPacket.data, "%s\nOFF\n", user->GetUsername().c_str());
						for (auto& other : this->m_users)
						{
							if (other.first != perClient->addr.sin_addr.S_un.S_addr)
							{
								_SendPacket_(other.second, sendPacket, other.second->m_io);
							}
						}
					}
					/* 释放连接 */
					this->m_users.erase(perClient->addr.sin_addr.S_un.S_addr);

					closesocket(perClient->client);
					this->m_needRelease[perClient->client] = false;
					this->m_needRelease.erase(perClient->client);
					delete perClient;
					delete perIO;
				}
				this->m_releaseLocker.unlock();
				this->m_userLocker.unlock();
				continue;
			}
			else if (perIO != nullptr && perClient != nullptr)
			{
				switch (perIO->opType)
				{
				case OPERATION_TYPE::RECV_POSTED:
				{
					perIO->buffer[bytesTransferred] = 0x0;
					std::memcpy((LPCH)&recvPacket, (LPCH)perIO->buffer, sizeof(Packet));
					_AnalyzePacket_(perClient->addr, recvPacket, perIO);
				}
				break;

				case OPERATION_TYPE::SEND_POSTED:
				{
					perIO->sendBytes += bytesTransferred;
					if (perIO->sendBytes < perIO->totalBytes)
					{
						/* 数据未发送完毕 继续投递 */
						fprintf(stdout, "数据未发送完毕 继续投递\n");
						ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
						perIO->dataBuf.buf = perIO->buffer + bytesTransferred;
						perIO->dataBuf.len = perIO->totalBytes - perIO->sendBytes;

						if (WSASend(perClient->client, &(perIO->dataBuf), 1, &sendBytes, 0,
							&(perIO->overlapped), NULL) == SOCKET_ERROR)
						{
							if (WSAGetLastError() != ERROR_IO_PENDING
								&& perClient != nullptr)
							{
								this->m_userLocker.lock();
								this->m_releaseLocker.lock();
								if (this->m_needRelease[perClient->client])
								{
									HUser user = this->m_users[perClient->addr.sin_addr.S_un.S_addr];
									if (user != nullptr)
									{
										sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
										sprintf(sendPacket.data, "%s\nOFF\n", user->GetUsername().c_str());
										for (auto& other : this->m_users)
										{
											if (other.first != perClient->addr.sin_addr.S_un.S_addr)
											{
												_SendPacket_(other.second, sendPacket, other.second->m_io);
											}
										}
									}
									/* 释放连接 */
									this->m_users.erase(perClient->addr.sin_addr.S_un.S_addr);

									closesocket(perClient->client);
									this->m_needRelease[perClient->client] = false;
									this->m_needRelease.erase(perClient->client);
									delete perClient;
									delete perIO;
								}
								this->m_releaseLocker.unlock();
								this->m_userLocker.unlock();
								continue;
							}
						}
					}
					else
					{
						// 初始化I/O操作结构体
						ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
						perIO->dataBuf.len = DATA_BUFSIZE;
						perIO->dataBuf.buf = perIO->buffer;
						perIO->opType = OPERATION_TYPE::RECV_POSTED;
						flags = 0;

						// 接收数据，放到PerIoData中
						// 而PerIoData又通过工作线程中的ServerWorkerThread函数取出
						if (WSARecv(perClient->client, &(perIO->dataBuf), 1, &recvBytes, &flags,
							&(perIO->overlapped), NULL) == SOCKET_ERROR)
						{
							if (WSAGetLastError() != ERROR_IO_PENDING
								&& perClient != nullptr)
							{
								this->m_userLocker.lock();
								this->m_releaseLocker.lock();
								if (this->m_needRelease[perClient->client])
								{
									HUser user = this->m_users[perClient->addr.sin_addr.S_un.S_addr];
									if (user != nullptr)
									{
										sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
										sprintf(sendPacket.data, "%s\nOFF\n", user->GetUsername().c_str());
										for (auto& other : this->m_users)
										{
											if (other.first != perClient->addr.sin_addr.S_un.S_addr)
											{
												_SendPacket_(other.second, sendPacket, other.second->m_io);
											}
										}
									}
									/* 释放连接 */
									this->m_users.erase(perClient->addr.sin_addr.S_un.S_addr);

									closesocket(perClient->client);
									this->m_needRelease[perClient->client] = false;
									this->m_needRelease.erase(perClient->client);
									delete perClient;
									delete perIO;
								}
								this->m_releaseLocker.unlock();
								this->m_userLocker.unlock();
								continue;
							}
						}
					}
				}
				break;

				default:
					break;
				}
			}
		}
	}
}

void Server::_ServerAcceptThread_()
{
	SockaddrIn clientAddr;
	int clientAddrLen = sizeof(SockaddrIn);
	LPPER_HANDLE_DATA perClient;
	LPPER_IO_OPERATION_DATA perIO;
	DWORD recvBytes;
	DWORD flags;
	try
	{
		while (!this->m_errorOccured && this->m_isServerOn)
		{
			Socket client
				= WSAAccept(this->m_serverSocket,
				(LPSOCKADDR)&clientAddr,
					&clientAddrLen,
					NULL,
					0);
			if (client == SOCKET_ERROR)
			{
				closesocket(this->m_serverSocket);
				WSACleanup();
				this->m_errorOccured = true;
				throw std::exception("网络异常：请求连接断开。\n");
				continue;
			}

			this->m_userLocker.lock();
			if (this->m_users[clientAddr.sin_addr.S_un.S_addr]
				!= nullptr)
			{
				delete m_users[clientAddr.sin_addr.S_un.S_addr];
			}
			this->m_users[clientAddr.sin_addr.S_un.S_addr]
				= new User{ client, clientAddr };
			this->m_userLocker.unlock();

			this->m_releaseLocker.lock();
			this->m_needRelease[client] = true;
			this->m_releaseLocker.unlock();

			// 分配并设置Socket句柄结构体
			if ((perClient = new PER_HANDLE_DATA{ }) == nullptr)
			{
				throw std::exception("内存不足。\n");
			}
			perClient->client = client;
			perClient->addr = clientAddr;

			fprintf(stdout, "收到%s的连接请求\n", inet_ntoa(perClient->addr.sin_addr));

			// 将与客户端进行通信的套接字Accept与完成端口CompletionPort相关联
			if (CreateIoCompletionPort(
				(HANDLE)client, this->m_completionPort, (ULONG_PTR)perClient, 0
			) == NULL)
			{
				throw std::exception("网络异常：完成端口绑定失败。\n");
			}

			// 为I/O操作结构体分配内存空间
			//if ((perIO = (LPPER_IO_OPERATION_DATA)GlobalAlloc(
			//	GPTR, sizeof(PER_IO_OPERATION_DATA)
			//)) == NULL)
			if ((perIO = new PER_IO_OPERATION_DATA{ }) == nullptr)
			{
				throw std::exception("内存不足。\n");
			}

			// 初始化I/O操作结构体
			ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
			perIO->dataBuf.len = DATA_BUFSIZE;
			perIO->dataBuf.buf = perIO->buffer;
			perIO->opType = OPERATION_TYPE::RECV_POSTED;
			flags = 0;

			// 接收数据，放到PerIoData中
			// 而PerIoData又通过工作线程中的ServerWorkerThread函数取出
			if (WSARecv(client, &(perIO->dataBuf), 1, &recvBytes, &flags,
				&(perIO->overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					// throw std::exception("网络异常：投递入境包失败。\n");
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		fprintf(stdout, "%s\n", e.what());
		/* 清理所有的通信线程 */
		std::for_each(this->m_workers.begin(), this->m_workers.end(),
			[](Thread& worker) {
			if (worker.joinable())
				worker.join();
		});
		this->m_users.clear();
		this->m_isServerOn = false;
	}
}

void Server::_BeatThread_()
{
	Packet sendPacket;
	sendPacket.type = PacketType::INVALID;
	while (!this->m_errorOccured && this->m_isServerOn)
	{
		Sleep(3000);

		this->m_userLocker.lock();
		/* 设置发送对象 */
		std::for_each(m_users.begin(), m_users.end(),
			[&sendPacket, this](const std::pair<ULONG, HUser>& user) {
			DWORD sendBytes;
			LPPER_IO_OPERATION_DATA perIO = user.second->m_io;
			ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
			std::memcpy((LPCH)perIO->buffer, (LPCH)&sendPacket, sizeof(Packet));
			perIO->dataBuf.buf = perIO->buffer;
			perIO->dataBuf.len = sizeof(Packet);
			perIO->opType = OPERATION_TYPE::SEND_POSTED;
			perIO->sendBytes = 0;
			perIO->totalBytes = sizeof(Packet);

			if (WSASend(user.second->m_client, &(perIO->dataBuf), 1, &sendBytes, 0,
				&(perIO->overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					fprintf(stdout, "IP=%s掉线\n", inet_ntoa(user.second->m_clientAddr.sin_addr));
					this->m_releaseLocker.lock();
					if (this->m_needRelease[user.second->m_client])
					{
						/* 释放连接 */
						this->m_users.erase(user.second->m_clientAddr.sin_addr.S_un.S_addr);

						closesocket(user.second->m_client);
						this->m_needRelease[user.second->m_client] = false;
						this->m_needRelease.erase(user.second->m_client);
					}
					this->m_releaseLocker.unlock();
				}
			}
		});
		this->m_userLocker.unlock();
	}
}
