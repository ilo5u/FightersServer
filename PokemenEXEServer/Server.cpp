#include "stdafx.h"
#include "Database.h"

#define DEBUG

typedef Packet::Type PacketType;

constexpr int  PORT = 27893; // 默认端口号

constexpr int  INIT_SUCCESS        = 0x00000000;
constexpr int  INIT_DATABASE_ERROR = 0xFFFFFFFE;
constexpr int  INIT_NETWORK_ERROR  = 0xFFFFFFFF;

constexpr auto DATABASE_USER     = "root";
constexpr auto DATABASE_PASSWORD = "19981031";
constexpr auto DATABASE_NAME     = "server";

#define NUMBER_OF_POKEMEN_COLUMNS 14

/// <summary>
/// 按换行符进行字符串切割
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
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

/// <summary>
/// 依据给定字符进行字符串切割
/// </summary>
/// <param name="data"></param>
/// <param name="ch"></param>
/// <returns></returns>
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
	m_onlineUsers(), m_onlineUserLocker(),
	m_rankedUsers(), m_rankedUserLocker(),
	m_completionPort(nullptr), 
	m_accepter(), m_workers(),
	m_errorOccured(false),
	m_isServerOn(false)
{
}

Server::~Server()
{
	this->m_isServerOn = false;
	if (this->m_accepter.joinable())
		this->m_accepter.join();

	delete this->m_hDatabase;
}

/// <summary>
/// 初始化服务器
/// </summary>
/// <returns></returns>
int Server::Init()
{
	if (!_InitDatabase_())
		return INIT_DATABASE_ERROR;

	/* 加载所有用户记录 */
	_LoadRankedUsers_();

	if (!_InitNetwork_())
		return INIT_NETWORK_ERROR;
	return INIT_SUCCESS;
}

/// <summary>
/// 运行服务器
/// </summary>
/// <returns></returns>
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

/// <summary>
/// 获取已连接的IP主机
/// </summary>
/// <returns></returns>
String Server::GetClients() const
{
	String queryResult;
	char   querySingle[BUFLEN];
	int    id = 0;
	for (const auto& user : this->m_onlineUsers)
	{
		if (user.second != nullptr)
		{
			++id;
			SockaddrIn addr;
			addr.sin_addr.S_un.S_addr = user.first;
			sprintf(querySingle, "用户%d: IP=%s\n", id, inet_ntoa(addr.sin_addr));
			queryResult += querySingle;
		}
	}

	if (queryResult.size() == 0)
	{
		queryResult = "无用户连接。\n";
	}
	return queryResult;
}

/// <summary>
/// 初始化数据库
/// </summary>
/// <returns></returns>
bool Server::_InitDatabase_()
{
	return m_hDatabase->Connect(DATABASE_USER, DATABASE_PASSWORD, DATABASE_NAME);
}

/// <summary>
/// 初始化网络
/// </summary>
/// <returns></returns>
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

#define LOAD_ALL_USERS_QUERY "select name,numberOfPokemens,rounds,wins from users"
/// <summary>
/// 加载排行榜
/// </summary>
void Server::_LoadRankedUsers_()
{
	char queryString[BUFSIZ];
	sprintf(queryString, LOAD_ALL_USERS_QUERY);

	Strings queryResult = this->m_hDatabase->Select(queryString, 4);
	this->m_rankedUserLocker.lock();
	for (const auto& userInfos : queryResult)
	{
		Strings elems = SplitData(userInfos.c_str(), '\n');
		if (std::atoi(elems[1].c_str()) > 0)
		{ /* 只导入拥有小精灵的用户 */
			this->m_rankedUsers.push_back(
				new User{
					elems[0],
					std::atoi(elems[1].c_str()),
					std::atoi(elems[2].c_str()),
					std::atoi(elems[3].c_str())
				}
			);
		}
	}
	this->m_rankedUserLocker.unlock();
}

/// <summary>
/// 分析数据包类型
/// </summary>
/// <param name="client"></param>
/// <param name="recv"></param>
/// <returns></returns>
bool Server::_AnalyzePacket_(LPPER_HANDLE_DATA client, const Packet& recv)
{
	printf("收到数据包类型=%d\n", (int)recv.type);
	switch (recv.type)
	{
	case PacketType::LOGIN_REQUEST:
		_DealWithLogin_(client, recv.data);
		return true;

	case PacketType::LOGON_REQUEST:
		_DealWithLogon_(client, recv.data);
		return true;

	case PacketType::GET_ONLINE_USERS:
		_DealWithGetOnlineUsers_(client, recv.data);
		return true;

	case PacketType::LOGOUT:
		_DealWithLogout_(client);
		return true;

	case PacketType::PVE_RESULT:
		_DealWithPVEResult_(client, recv.data);
		return true;

	case PacketType::PROMOTE_POKEMEN:
		_DealWithPromotePokemen_(client, recv.data);
		return true;

	case PacketType::ADD_POKEMEN:
		_DealWithAddPokemen_(client);
		return true;

	case PacketType::SUB_POKEMEN:
		_DealWithSubPokemen_(client, recv.data);
		return true;

	case PacketType::GET_POKEMENS_BY_USER:
		_DealWithGetPokemensByUser_(client, recv.data);
		return true;

	case PacketType::UPDATE_RANKLIST:
		_DealWithUpdateRanklist_(client);
		return true;

	case PacketType::PVP_REQUEST:
		_DealWithPVPRequest_(client, recv.data);
		return true;

	case PacketType::PVP_CANCEL:
		_DealWithPVPCancel_(client);
		return true;

	case PacketType::PVP_ACCEPT:
		_DealWithPVPAccept_(client, recv.data);
		return true;

	case PacketType::PVP_BUSY:
		_DealWithPVPBusy_(client, recv.data);
		return true;

	case PacketType::PVP_MESSAGE:
		_DealWithPVPMessage_(client, recv.data);
		return true;

	case PacketType::PVP_RESULT:
		_DealWithPVPResult_(client, recv.data);
		return true;

	case PacketType::PVP_BATTLE:
		_DealWithPVPBattle_(client, recv.data);
		return true;

	default:
		return false;
	}
}

/* +++++++++++++++++++++++ 数据库查询语句块 +++++++++++++++++++++++ */
#define SELECT_USER_QUERYSTRING "\
select numberOfPokemens,rounds,wins,tops from users \
where name='%s' and password='%s'"

#define NUMBER_OF_USER_COLUMNS 4

#define INSERT_POKEMEN_QUERYSTRING 	"\
insert into pokemens(user,type,name,\
hpoints,attack,defense,agility,interva,critical,hitratio,parryratio,\
career,exp,level) values('%s',%d,'%s',\
%d,%d,%d,%d,%d,%d,%d,%d,\
%d,%d,%d)"

#define SELECT_POKEMEN_BY_USER_QUERYSTRING 	"\
select identity,type,name,\
hpoints,attack,defense,agility,interva,critical,hitratio,parryratio,\
career,exp,level from pokemens where user='%s'"

#define SELECT_POKEMEN_BY_ID_QUERYSTRING 	"\
select identity,type,name,\
hpoints,attack,defense,agility,interva,critical,hitratio,parryratio,\
career,exp,level from pokemens where identity='%d'"

#define UPDATE_POKEMEN_QUERYSTRING "\
update pokemens set hpoints=%d,attack=%d,defense=%d,agility=%d,interva=%d,critical=%d,hitratio=%d,parryratio=%d,\
career=%d,exp=%d,level=%d where identity=%d"

#define POKEMEN_ALL_PROPERTIES "%d\n%d\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n"
/* +++++++++++++++++++++++ 数据库查询语句块 +++++++++++++++++++++++ */

const static int initNumberOfPokemens = 3;

/// <summary>
/// 用户登陆
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithLogin_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult; // 查询结果
	char    szQuery[BUFLEN];

	Strings userInfos = SplitData(data);
	Packet  sendPacket;

	int userId = client->addr.sin_addr.S_un.S_addr;
	printf("请求接入 %s\n", inet_ntoa(client->addr.sin_addr));
	try
	{
		sprintf(szQuery, SELECT_USER_QUERYSTRING,
			userInfos[0].c_str(), userInfos[1].c_str());
		queryResult = m_hDatabase->Select(szQuery, NUMBER_OF_USER_COLUMNS);

		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();
		if (onlineUser != nullptr)
		{
			// 反馈登陆信息
			bool existed = true;
			this->m_onlineUserLocker.lock();
			if (std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
				[&userInfos](const std::pair<ULONG, HOnlineUser>& perUser) {
				if (perUser.second != nullptr)
				{
					return perUser.second->username == userInfos[0];
				}
				else
				{
					return false;
				}
			}) == this->m_onlineUsers.end())
			{
				existed = false;
			}
			this->m_onlineUserLocker.unlock();
			Strings queryElems;
			if (existed || queryResult.empty())
			{
				sendPacket.type = PacketType::LOGIN_FAILED;
				printf("登陆失败 %s %s\n", inet_ntoa(client->addr.sin_addr), userInfos[0].c_str());
			}
			else
			{
				queryElems = SplitData(queryResult[0].c_str());
				sendPacket.type = PacketType::LOGIN_SUCCESS;
				onlineUser->SetUsername(userInfos[0]);
				sprintf(sendPacket.data, "%s", queryElems[0].c_str());
			}
			_SendPacket_(onlineUser, sendPacket);

			// 反馈在线用户以及小精灵信息
			if (sendPacket.type == PacketType::LOGIN_SUCCESS)
			{
				_OnLoginSuccessCallBack(onlineUser, userInfos, queryElems);
			}
		}
	}
	catch (const std::exception& e)
	{
		printf("用户登录时发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

void Server::_OnLoginSuccessCallBack(HOnlineUser onlineUser, const Strings& userInfos, const Strings& queryElems)
{
	Strings queryResult;
	char    szQuery[BUFLEN];
	Packet  sendPacket;

	try
	{
		onlineUser->username = userInfos[0];
		onlineUser->numberOfPokemens = std::atoi(queryElems[0].c_str());
		onlineUser->rounds = std::atoi(queryElems[1].c_str());
		onlineUser->wins = std::atoi(queryElems[2].c_str());
		onlineUser->tops = std::atoi(queryElems[3].c_str());

		printf("登陆成功 用户：%s 精灵数目：%d 比赛次数：%d 获胜场数：%d 高阶精灵：%d\n",
			onlineUser->username.c_str(), onlineUser->numberOfPokemens, onlineUser->rounds, onlineUser->wins, onlineUser->tops);

		if (onlineUser->numberOfPokemens == 0)
		{  /* 用户第一次登陆，随机生成三只小精灵 */
			printf("发放三只小精灵 %s\n", inet_ntoa(onlineUser->m_clientAddr.sin_addr));
			for (int i = 0; i < initNumberOfPokemens; ++i)
			{
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
			onlineUser->numberOfPokemens = initNumberOfPokemens;

			/* 将该用户插入排行榜 */
			this->m_rankedUserLocker.lock();
			this->m_rankedUsers.push_back(
				new User{
					onlineUser->username,
					onlineUser->numberOfPokemens,
					onlineUser->rounds,
					onlineUser->wins,
					onlineUser->tops
				}
			);
			this->m_rankedUserLocker.unlock();
			this->_OnRenewRanklistCallBack_(onlineUser);
		}
		else
		{
			/* 刷新排行榜 */
			this->m_rankedUserLocker.lock();
			RankedUsers::iterator currentUser = std::find_if(this->m_rankedUsers.begin(), this->m_rankedUsers.end(),
				[&onlineUser](const HUser perUser) {
				return perUser->username == onlineUser->username;
			});
			this->m_rankedUserLocker.unlock();
			(*currentUser)->numberOfPokemens = onlineUser->numberOfPokemens;
			(*currentUser)->rounds = onlineUser->rounds;
			(*currentUser)->wins = onlineUser->wins;
			(*currentUser)->tops = onlineUser->tops;
		}

		this->_OnUpdatePokemensCallBack_(onlineUser); // 反馈小精灵
		this->_OnUpdateRanklistCallBack_(onlineUser); // 反馈排行榜

		/* 向其他在线用户反馈该用户的在线状态 */
		sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
		sprintf(sendPacket.data, "%s\nON\n", userInfos[0].c_str());
		this->m_onlineUserLocker.lock();
		for (const auto& otherUser : this->m_onlineUsers)
		{
			if (otherUser.first != onlineUser->GetUserID()
				&& otherUser.second != nullptr)
			{
				_SendPacket_(otherUser.second, sendPacket);
			}
		}
		this->m_onlineUserLocker.unlock();
	}
	catch (const std::exception& e)
	{
		printf("用户登录成功时发生异常：%s\n", e.what());
	}
}

/// <summary>
/// 用户注册
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithLogon_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult; // 查询结果
	char    szQuery[BUFLEN];

	Strings userInfos = SplitData(data);
	Packet  sendPacket;

	int userId = client->addr.sin_addr.S_un.S_addr;
	printf("注册请求 %s\n", inet_ntoa(client->addr.sin_addr));
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();
		if (onlineUser != nullptr)
		{
			sprintf(szQuery,
				"insert into users values('%s','%s',0, 0, 0, 0)",
				userInfos[0].c_str(), userInfos[1].c_str()
			);

			if (this->m_hDatabase->Insert(szQuery))
			{
				sendPacket.type = PacketType::LOGON_SUCCESS;
				printf("注册成功 %s %s IP=%s\n", userInfos[0].c_str(), userInfos[1].c_str(),
					inet_ntoa(client->addr.sin_addr));
			}
			else
			{
				sendPacket.type = PacketType::LOGON_FAILED;
				printf("注册失败 %s %s IP=%s\n", userInfos[0].c_str(), userInfos[1].c_str(),
					inet_ntoa(client->addr.sin_addr));
			}
			_SendPacket_(onlineUser, sendPacket);
		}
	}
	catch (const std::exception& e)
	{
		printf("注册时发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

/// <summary>
/// 用户注销
/// </summary>
/// <param name="client"></param>
void Server::_DealWithLogout_(LPPER_HANDLE_DATA client)
{
	try
	{
		printf("用户注销 IP=%s\n", inet_ntoa(client->addr.sin_addr));
		this->_OnConnectionLostCallBack_(client, nullptr);
	}
	catch (const std::exception& e)
	{
		printf("用户注销发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

/// <summary>
/// 获取在线用户
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithGetOnlineUsers_(LPPER_HANDLE_DATA client, const char data[])
{
	Packet sendPacket;
	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();
		if (onlineUser != nullptr)
		{
			printf("用户请求获取在线用户 IP=%s\n", inet_ntoa(client->addr.sin_addr));

			char szUserNames[BUFLEN];
			szUserNames[0] = 0x0;
			int  cnt = 0;
			sendPacket.type = PacketType::SET_ONLINE_USERS;
			bool hasSend = false;
			for (const auto& otherUser : this->m_onlineUsers)
			{ /* 将最多20个用户分为一组打包发送 */
				if (otherUser.first != userId
					&& otherUser.second != nullptr)
				{
					++cnt;
					sprintf(szUserNames + std::strlen(szUserNames),
						"%s\n", otherUser.second->GetUsername().c_str()
					);
					if (cnt == 20)
					{
						sprintf(sendPacket.data, "20\n%s", szUserNames);
						_SendPacket_(onlineUser, sendPacket);
						hasSend = true;

						cnt = 0;
						szUserNames[0] = 0x0;
					}
				}
			}
			if (cnt > 0)
			{
				sprintf(sendPacket.data, "%d\n%s", cnt, szUserNames);
				_SendPacket_(onlineUser, sendPacket);
				hasSend = true;
			}
			if (!hasSend)
			{
				this->_RecvPacket_(onlineUser);
			}
		}
	}
	catch (const std::exception& e)
	{
		printf("用户获取在线用户时发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

/// <summary>
/// 处理用户与电脑战斗的结果
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithPVEResult_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult; // 查询结果
	char    szQuery[BUFLEN];
	Packet  sendPacket;
	Strings infos = SplitData(data);

	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();
		if (onlineUser != nullptr)
		{
			if (infos.size() != 17)
			{
				this->_RecvPacket_(onlineUser);
				return;
			}

			++onlineUser->rounds;
			if (infos[0].compare("F") == 0)
				++onlineUser->wins;

			/* 处理对应的精灵 */
			Pokemen::Property prop{
				std::atoi(infos[4].c_str()), infos[5].c_str(),
				std::atoi(infos[6].c_str()), std::atoi(infos[7].c_str()), std::atoi(infos[8].c_str()), std::atoi(infos[9].c_str()),
				std::atoi(infos[10].c_str()), std::atoi(infos[11].c_str()), std::atoi(infos[12].c_str()), std::atoi(infos[13].c_str()),
				std::atoi(infos[15].c_str()), std::atoi(infos[16].c_str()),
				std::atoi(infos[3].c_str())
			};
			Pokemens::iterator pokemen = std::find_if(onlineUser->m_pokemens.begin(),
				onlineUser->m_pokemens.end(),
				[&prop](const Pokemen::Pokemen& temp) {
				return temp.GetId() == prop.m_id;
			});
			if (pokemen != onlineUser->m_pokemens.end()
				&& pokemen->GetLevel() < 15)
			{ /* 等于15级则无需更新 */
				pokemen->RenewProperty(prop, std::atoi(infos[14].c_str()));
				sprintf(szQuery,
					UPDATE_POKEMEN_QUERYSTRING,
					prop.m_hpoints, prop.m_attack, prop.m_defense, prop.m_agility,
					prop.m_interval, prop.m_critical, prop.m_hitratio, prop.m_parryratio,
					std::atoi(infos[14].c_str()), prop.m_exp, prop.m_level,
					prop.m_id
				);
				this->m_hDatabase->Update(szQuery);

				/* 回传更新后小精灵 */
				sendPacket.type = PacketType::UPDATE_POKEMENS;
				sprintf(sendPacket.data,
					POKEMEN_ALL_PROPERTIES,
					pokemen->GetId(), (int)pokemen->GetType(), pokemen->GetName().c_str(),
					pokemen->GetHpoints(), pokemen->GetAttack(), pokemen->GetDefense(), pokemen->GetAgility(),
					pokemen->GetInterval(), pokemen->GetCritical(), pokemen->GetHitratio(), pokemen->GetParryratio(),
					pokemen->GetCareer(), pokemen->GetExp(), pokemen->GetLevel()
				);
				_SendPacket_(onlineUser, sendPacket);

				/* 处理高阶精灵的数目 */
				if (pokemen->GetLevel() == 15)
				{
					++onlineUser->tops;
				}
			}

			/* 更新数据库 */
			sprintf(szQuery,
				"update users set rounds=%d,wins=%d,tops=%d where name='%s'",
				onlineUser->rounds, onlineUser->wins, onlineUser->tops, onlineUser->username.c_str());
			this->m_hDatabase->Update(szQuery);

			/* 更新排行榜中的数据 */
			this->m_rankedUserLocker.lock();
			RankedUsers::iterator rankedUser = std::find_if(this->m_rankedUsers.begin(), this->m_rankedUsers.end(),
				[&onlineUser](HUser perUser) {
				return perUser->username == onlineUser->username;
			});
			(*rankedUser)->rounds = onlineUser->rounds;
			(*rankedUser)->wins = onlineUser->wins;
			(*rankedUser)->tops = onlineUser->tops;
			this->m_rankedUserLocker.unlock();

			/* 为其他所有用户更新排行榜 */
			this->_OnRenewRanklistCallBack_(onlineUser);
		}
	}
	catch (const std::exception& e)
	{
		printf("接收PVE结果时发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

/// <summary>
/// 小精灵转职
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithPromotePokemen_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult; // 查询结果
	char    szQuery[BUFLEN];
	Packet  sendPacket;
	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{ 
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();
		if (onlineUser != nullptr)
		{
			int pokemenId = 0;
			int careerType = 0;
			sscanf(data, "%d\n%d\n", &pokemenId, &careerType);
			Pokemens::iterator pokemen = std::find_if(onlineUser->m_pokemens.begin(),
				onlineUser->m_pokemens.end(),
				[&pokemenId](const Pokemen::Pokemen& temp){
					return temp.GetId() == pokemenId;
			});
			if (pokemen != onlineUser->m_pokemens.end() && careerType < 3)
			{
				/* 转职 */
				pokemen->Promote(careerType);
				sprintf(szQuery,
					UPDATE_POKEMEN_QUERYSTRING,
					pokemen->GetHpoints(), pokemen->GetAttack(), pokemen->GetDefense(), pokemen->GetAgility(),
					pokemen->GetInterval(), pokemen->GetCritical(), pokemen->GetHitratio(), pokemen->GetParryratio(),
					pokemen->GetCareer(), pokemen->GetExp(), pokemen->GetLevel(),
					pokemenId
				);
				this->m_hDatabase->Update(szQuery);

				/* 回传更新后小精灵 */
				sendPacket.type = PacketType::UPDATE_POKEMENS;
				sprintf(sendPacket.data,
					POKEMEN_ALL_PROPERTIES,
					pokemen->GetId(), (int)pokemen->GetType(), pokemen->GetName().c_str(),
					pokemen->GetHpoints(), pokemen->GetAttack(), pokemen->GetDefense(), pokemen->GetAgility(),
					pokemen->GetInterval(), pokemen->GetCritical(), pokemen->GetHitratio(), pokemen->GetParryratio(),
					pokemen->GetCareer(), pokemen->GetExp(), pokemen->GetLevel()
				);
				_SendPacket_(onlineUser, sendPacket);
			}
			else
			{
				this->_RecvPacket_(onlineUser);
			}
		}
	}
	catch (const std::exception& e)
	{
		printf("用户转职精灵时发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

/// <summary>
/// 用户获得一只新的小精灵
/// </summary>
/// <param name="client"></param>
void Server::_DealWithAddPokemen_(LPPER_HANDLE_DATA client)
{
	Strings queryResult; // 查询结果
	char    szQuery[BUFLEN];
	Packet  sendPacket;
	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();

		if (onlineUser != nullptr)
		{
			/* 当用户断开连接时也将进行精灵回收 */
			onlineUser->m_needRemove = false;

			/* 为用户新发放一个小精灵 */
			Pokemen::Pokemen pokemen{ PokemenType::DEFAULT, 0x1 };
			sprintf(szQuery, INSERT_POKEMEN_QUERYSTRING,
				onlineUser->username.c_str(),
				(int)pokemen.GetType(), pokemen.GetName().c_str(),
				pokemen.GetHpoints(), pokemen.GetAttack(),
				pokemen.GetDefense(), pokemen.GetAgility(),
				pokemen.GetInterval(), pokemen.GetCritical(),
				pokemen.GetHitratio(), pokemen.GetParryratio(),
				pokemen.GetCareer(),
				pokemen.GetExp(), pokemen.GetLevel()
			);
			this->m_hDatabase->Insert(szQuery);

			++onlineUser->numberOfPokemens;
			sprintf(szQuery,
				"update users set numberOfPokemens=%d where name='%s'",
				onlineUser->numberOfPokemens, onlineUser->username.c_str()
			);
			this->m_hDatabase->Update(szQuery);

			/* 反馈该用户的最后一个小精灵 */
			sendPacket.type = PacketType::ADD_POKEMEN;
			sprintf(szQuery, SELECT_POKEMEN_BY_USER_QUERYSTRING, onlineUser->username.c_str());
			queryResult = this->m_hDatabase->Select(szQuery, NUMBER_OF_POKEMEN_COLUMNS);
			int lastPokemen = (int)queryResult.size() - 1;

			onlineUser->InsertAPokemen(queryResult[lastPokemen]);
			sprintf(sendPacket.data, queryResult[lastPokemen].c_str());
			_SendPacket_(onlineUser, sendPacket);

			/* 更新用户列表中的数据 */
			this->m_rankedUserLocker.lock();
			(*std::find_if(this->m_rankedUsers.begin(), this->m_rankedUsers.end(), [&onlineUser](HUser perUser) {
				return perUser->username == onlineUser->username;
			}))->numberOfPokemens = onlineUser->numberOfPokemens;
			this->m_rankedUserLocker.unlock();

			/* 向其他所有在线用户发送更新排行榜的通知 */
			this->_OnRenewRanklistCallBack_(onlineUser);
		}
	}
	catch (std::exception& e)
	{
		printf("用户获取新精灵时发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

#define REMOVE_POKEMEN_QUERY "delete from pokemens where identity=%d"
/// <summary>
/// 用户抛弃一个小精灵
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithSubPokemen_(LPPER_HANDLE_DATA client, const char data[])
{
	Packet  sendPacket;
	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();

		if (onlineUser != nullptr)
		{
			int removeId = 0;
			sscanf(data, "%d", &removeId);
			if (!onlineUser->m_needRemove && removeId == 0)
			{
				onlineUser->m_needRemove = true;
				this->_RecvPacket_(onlineUser);
			}
			else if (removeId > 0)
			{
				this->_OnRemovePokemenCallBack_(onlineUser, removeId);
			}
			else
			{
				this->_RecvPacket_(onlineUser);
			}
		}
	}
	catch (std::exception& e)
	{
		printf("用户抛弃精灵时发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

/// <summary>
/// 查询其他用户的小精灵
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithGetPokemensByUser_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult; // 查询结果
	char    szQuery[BUFLEN];
	Packet  sendPacket;
	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();

		if (onlineUser != nullptr)
		{
			sprintf(szQuery, SELECT_POKEMEN_BY_USER_QUERYSTRING, data);
			queryResult = this->m_hDatabase->Select(szQuery, NUMBER_OF_POKEMEN_COLUMNS);

			sendPacket.type = PacketType::SET_POKEMENS_BY_USER;
			for (const auto& pokemen : queryResult)
			{
				sprintf(sendPacket.data, pokemen.c_str());
				_SendPacket_(onlineUser, sendPacket);
			}

			sendPacket.type = PacketType::SET_POKEMENS_OVER;
			_SendPacket_(onlineUser, sendPacket);
		}
	}
	catch (std::exception& e)
	{
		printf("用户查询其他用户的精灵时发生异常：%s IP=%s\n", e.what(), inet_ntoa(client->addr.sin_addr));
	}
}

/// <summary>
/// 获取排行榜
/// </summary>
/// <param name="client"></param>
void Server::_DealWithUpdateRanklist_(LPPER_HANDLE_DATA client)
{
	int userId = client->addr.sin_addr.S_un.S_addr;
	this->m_onlineUserLocker.lock();
	HOnlineUser onlineUser = this->m_onlineUsers[userId];
	this->m_onlineUserLocker.unlock();

	if (onlineUser != nullptr)
	{
		this->_OnUpdateRanklistCallBack_(onlineUser);
	}
}

/// <summary>
/// 用户请求在线对战
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithPVPRequest_(LPPER_HANDLE_DATA client, const char data[])
{
	int userId = client->addr.sin_addr.S_un.S_addr;
	this->m_onlineUserLocker.lock();
	HOnlineUser onlineUser = this->m_onlineUsers[userId];
	this->m_onlineUserLocker.unlock();

	Packet sendPacket;
	Strings userInfos = SplitData(data);
	if (onlineUser != nullptr)
	{
		printf("请求与%s对战 IP=%s\n", data, inet_ntoa(client->addr.sin_addr));
		if (userInfos.size() == 0)
		{ /* 无效数据包 */
			this->_RecvPacket_(onlineUser);
			return;
		}

		bool existed = false;
		this->m_onlineUserLocker.lock();
		OnlineUsers::iterator oppoent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
			[&userInfos](const std::pair<ULONG, HOnlineUser>& perUser) {
			if (perUser.second != nullptr)
			{
				return perUser.second->username == userInfos[0];
			}
			else
			{
				return false;
			}
		});
		if (oppoent != this->m_onlineUsers.end())
		{
			existed = true;
		}
		this->m_onlineUserLocker.unlock();

		if (existed)
		{
			if (!oppoent->second->m_opponent.empty())
			{
				sendPacket.type = PacketType::PVP_BUSY;
				sprintf(sendPacket.data, "%s", oppoent->second->username.c_str());

				this->_SendPacket_(onlineUser, sendPacket);
			}
			else
			{
				onlineUser->m_opponent = userInfos[0];
				sendPacket.type = PacketType::PVP_REQUEST;
				sprintf(sendPacket.data, "%s", onlineUser->username.c_str());

				this->_SendPacket_(oppoent->second, sendPacket);
				oppoent->second->m_opponent = onlineUser->username;
			}
		}
		this->_RecvPacket_(onlineUser);
	}
}

/// <summary>
/// 用户取消对战
/// </summary>
/// <param name="client"></param>
void Server::_DealWithPVPCancel_(LPPER_HANDLE_DATA client)
{
	int userId = client->addr.sin_addr.S_un.S_addr;
	this->m_onlineUserLocker.lock();
	HOnlineUser onlineUser = this->m_onlineUsers[userId];
	this->m_onlineUserLocker.unlock();

	Packet sendPacket;
	if (onlineUser != nullptr)
	{
		bool existed = false;
		this->m_onlineUserLocker.lock();
		String username = onlineUser->m_opponent;
		OnlineUsers::iterator oppoent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
			[&username](const std::pair<ULONG, HOnlineUser>& perUser) {
			if (perUser.second != nullptr)
			{
				return perUser.second->username == username;
			}
			else
			{
				return false;
			}
		});
		if (oppoent != this->m_onlineUsers.end())
		{
			existed = true;
		}
		this->m_onlineUserLocker.unlock();

		if (existed)
		{
			sendPacket.type = PacketType::PVP_CANCEL;
			sprintf(sendPacket.data, "%s", onlineUser->username.c_str());

			this->_SendPacket_(oppoent->second, sendPacket);

			/* 断开在线连接 */
			OnlineUsers::iterator opponent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
				[&onlineUser](const std::pair<ULONG, HOnlineUser>& perUser) {
				if (perUser.second != nullptr)
				{
					return perUser.second->GetOpponent() == onlineUser->GetUsername();
				}
				else
				{
					return false;
				}
			});
			if (opponent != this->m_onlineUsers.end())
			{
				/* 断掉在线对战的另一方 */
				oppoent->second->m_opponent.clear();
			}
			onlineUser->m_opponent.clear();
		}
		this->_RecvPacket_(onlineUser);
	}
}

/// <summary>
/// 用户接受对战
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithPVPAccept_(LPPER_HANDLE_DATA client, const char data[])
{
	int userId = client->addr.sin_addr.S_un.S_addr;
	this->m_onlineUserLocker.lock();
	HOnlineUser onlineUser = this->m_onlineUsers[userId];
	this->m_onlineUserLocker.unlock();

	Packet sendPacket;
	Strings userInfos = SplitData(data);
	if (onlineUser != nullptr)
	{
		if (userInfos.size() == 0)
		{ /* 无效数据包 */
			this->_RecvPacket_(onlineUser);
			return;
		}

		bool existed = false;
		this->m_onlineUserLocker.lock();
		OnlineUsers::iterator oppoent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
			[&userInfos](const std::pair<ULONG, HOnlineUser>& perUser) {
			if (perUser.second != nullptr)
			{
				return perUser.second->username == userInfos[0];
			}
			else
			{
				return false;
			}
		});
		if (oppoent != this->m_onlineUsers.end())
		{
			existed = true;
		}
		this->m_onlineUserLocker.unlock();

		if (existed)
		{
			/* 获取接受方的出战精灵 */
			onlineUser->m_opponent = userInfos[0];
			String pokemenInfos = onlineUser->PokemenAt(std::atoi(userInfos[1].c_str()));
			sendPacket.type = PacketType::PVP_ACCEPT;
			sprintf(sendPacket.data, "%s", pokemenInfos.c_str());

			this->_SendPacket_(oppoent->second, sendPacket);
			oppoent->second->m_opponent = onlineUser->username;
		}
		else
		{
			sendPacket.type = PacketType::PVP_CANCEL;
			this->_SendPacket_(onlineUser, sendPacket);
			onlineUser->m_opponent.clear();
		}
		this->_RecvPacket_(onlineUser);
	}
}

/// <summary>
/// 用户正忙
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithPVPBusy_(LPPER_HANDLE_DATA client, const char data[])
{
	int userId = client->addr.sin_addr.S_un.S_addr;
	this->m_onlineUserLocker.lock();
	HOnlineUser onlineUser = this->m_onlineUsers[userId];
	this->m_onlineUserLocker.unlock();

	Packet sendPacket;
	if (onlineUser != nullptr)
	{
		if (onlineUser->GetOpponent().empty())
		{
			this->_RecvPacket_(onlineUser);
			return;
		}

		bool existed = false;
		this->m_onlineUserLocker.lock();
		OnlineUsers::iterator oppoent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
			[&onlineUser](const std::pair<ULONG, HOnlineUser>& perUser) {
			if (perUser.second != nullptr)
			{
				return perUser.second->username == onlineUser->GetOpponent();
			}
			else
			{
				return false;
			}
		});
		if (oppoent != this->m_onlineUsers.end())
		{
			existed = true;
		}

		this->m_onlineUserLocker.unlock();

		if (existed)
		{
			sendPacket.type = PacketType::PVP_BUSY;
			sprintf(sendPacket.data, "%s", onlineUser->username.c_str());

			this->_SendPacket_(oppoent->second, sendPacket);
			oppoent->second->m_opponent.clear();
		}
		onlineUser->m_opponent.clear();
		this->_RecvPacket_(onlineUser);
	}
}

/// <summary>
/// 宿主发起开始信号
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithPVPBattle_(LPPER_HANDLE_DATA client, const char data[])
{
	int userId = client->addr.sin_addr.S_un.S_addr;
	this->m_onlineUserLocker.lock();
	HOnlineUser onlineUser = this->m_onlineUsers[userId];
	this->m_onlineUserLocker.unlock();

	Packet sendPacket;
	Strings userInfos = SplitData(data);
	if (onlineUser != nullptr)
	{
		if (userInfos.size() == 0)
		{
			this->_RecvPacket_(onlineUser);
			return;
		}

		bool existed = false;
		this->m_onlineUserLocker.lock();
		OnlineUsers::iterator oppoent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
			[&userInfos](const std::pair<ULONG, HOnlineUser>& perUser) {
			if (perUser.second != nullptr)
			{
				return perUser.second->username == userInfos[0];
			}
			else
			{
				return false;
			}
		});
		if (oppoent != this->m_onlineUsers.end())
		{
			existed = true;
		}
		this->m_onlineUserLocker.unlock();

		if (existed)
		{
			/* 获取发起方的出战精灵 */
			String pokemenInfos = onlineUser->PokemenAt(std::atoi(userInfos[1].c_str()));
			sendPacket.type = PacketType::PVP_BATTLE;
			sprintf(sendPacket.data, "%s", pokemenInfos.c_str());

			this->_SendPacket_(oppoent->second, sendPacket);
		}
		else
		{
			sendPacket.type = PacketType::PVP_CANCEL;
			this->_SendPacket_(onlineUser, sendPacket);
			onlineUser->m_opponent.clear();
		}
		this->_RecvPacket_(onlineUser);
	}
}

/// <summary>
/// 在线对战信息
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithPVPMessage_(LPPER_HANDLE_DATA client, const char data[])
{
	int userId = client->addr.sin_addr.S_un.S_addr;
	this->m_onlineUserLocker.lock();
	HOnlineUser onlineUser = this->m_onlineUsers[userId];
	this->m_onlineUserLocker.unlock();

	Packet sendPacket;
	if (onlineUser != nullptr)
	{
		bool existed = false;
		this->m_onlineUserLocker.lock();
		String username = onlineUser->m_opponent;
		OnlineUsers::iterator oppoent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
			[&username](const std::pair<ULONG, HOnlineUser>& perUser) {
			if (perUser.second != nullptr)
			{
				return perUser.second->username == username;
			}
			else
			{
				return false;
			}
		});
		if (oppoent != this->m_onlineUsers.end())
		{
			existed = true;
		}
		this->m_onlineUserLocker.unlock();

		if (existed)
		{
			sendPacket.type = PacketType::PVP_MESSAGE;
			sprintf(sendPacket.data, "%s", data);
			this->_SendPacket_(oppoent->second, sendPacket);
		}
		else
		{
			sendPacket.type = PacketType::PVP_CANCEL;
			this->_SendPacket_(onlineUser, sendPacket);
			onlineUser->m_opponent.clear();
		}
		this->_RecvPacket_(onlineUser);
	}
}

/// <summary>
/// 在线对战的结果
/// </summary>
/// <param name="client"></param>
/// <param name="data"></param>
void Server::_DealWithPVPResult_(LPPER_HANDLE_DATA client, const char data[])
{
	int userId = client->addr.sin_addr.S_un.S_addr;
	this->m_onlineUserLocker.lock();
	HOnlineUser onlineUser = this->m_onlineUsers[userId];
	this->m_onlineUserLocker.unlock();

	Packet sendPacket;
	if (onlineUser != nullptr)
	{
		bool existed = false;
		this->m_onlineUserLocker.lock();
		String username = onlineUser->m_opponent;
		OnlineUsers::iterator oppoent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
			[&username](const std::pair<ULONG, HOnlineUser>& perUser) {
			if (perUser.second != nullptr)
			{
				return perUser.second->username == username;
			}
			else
			{
				return false;
			}
		});
		if (oppoent != this->m_onlineUsers.end())
		{
			existed = true;
		}
		this->m_onlineUserLocker.unlock();

		if (existed)
		{
			sendPacket.type = PacketType::PVP_RESULT;
			sprintf(sendPacket.data, "%s", data);
			this->_SendPacket_(oppoent->second, sendPacket);

			/* 断开连接信息 */
			oppoent->second->m_opponent.clear();
		}
		onlineUser->m_opponent.clear();
		this->_RecvPacket_(onlineUser);
	}
}

/// <summary>
/// 连接丢失
/// </summary>
/// <param name="lostClient"></param>
/// <param name="lostIO"></param>
void Server::_OnConnectionLostCallBack_(LPPER_HANDLE_DATA lostClient, LPPER_IO_OPERATION_DATA lostIO)
{
	int lostId = lostClient->addr.sin_addr.S_un.S_addr;
	Packet sendPacket;

	this->m_onlineUserLocker.lock();
	this->m_releaseLocker.lock();
	if (this->m_needRelease[lostClient->client])
	{
		/* 通知其他用户更新在线用户列表 */
		HOnlineUser lostUser = this->m_onlineUsers[lostId];
		if (lostUser != nullptr)
		{
			printf("%s断开连接\n", inet_ntoa(lostClient->addr.sin_addr));

			sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
			sprintf(sendPacket.data, "%s\nOFF\n", lostUser->GetUsername().c_str());
			for (auto& otherUser : this->m_onlineUsers)
			{
				if (otherUser.first != lostId
					&& otherUser.second != nullptr)
				{
					_SendPacket_(otherUser.second, sendPacket);
				}
			}
		}

		/* 断线补偿处理 */
		OnlineUsers::iterator opponent = std::find_if(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
			[&lostUser](const std::pair<ULONG, HOnlineUser>& perUser) {
			if (perUser.second != nullptr)
			{
				return perUser.second->GetOpponent() == lostUser->GetUsername();
			}
			else
			{
				return false;
			}
		});
		if (opponent != this->m_onlineUsers.end())
		{
			/* 断掉在线对战的另一方 */
			sendPacket.type = PacketType::PVP_RESULT;
			sprintf(sendPacket.data, "F\n0\n");
			this->_SendPacket_(opponent->second, sendPacket);
			opponent->second->m_opponent.clear();
		}

		if (lostUser->m_needRemove)
		{
			/* 移除该用户的一个精灵 */
			this->_OnRemovePokemenCallBack_(lostUser, 0);
		}

		/* 释放连接 */
		this->m_onlineUsers.erase(lostId);
		delete lostUser;

		closesocket(lostClient->client);
		this->m_needRelease[lostClient->client] = false;
		delete lostClient;
		delete lostIO;
	}
	this->m_releaseLocker.unlock();
	this->m_onlineUserLocker.unlock();
}

/// <summary>
/// 获取用户的精灵
/// </summary>
/// <param name="onlineUser"></param>
void Server::_OnUpdatePokemensCallBack_(HOnlineUser onlineUser)
{
	Packet  sendPacket;
	char    szQuery[BUFSIZ];
	Strings queryResult;

	/* 反馈该用户所拥有的小精灵 */
	sendPacket.type = PacketType::UPDATE_POKEMENS;
	sprintf(szQuery, SELECT_POKEMEN_BY_USER_QUERYSTRING, onlineUser->username.c_str());
	queryResult = this->m_hDatabase->Select(szQuery, NUMBER_OF_POKEMEN_COLUMNS);
	for (const auto& pokemen : queryResult)
	{
		onlineUser->InsertAPokemen(pokemen);
		sprintf(sendPacket.data, pokemen.c_str());
		_SendPacket_(onlineUser, sendPacket);
	}
}

/// <summary>
/// 更新排行榜
/// </summary>
/// <param name="onlineUser"></param>
void Server::_OnRenewRanklistCallBack_(HOnlineUser onlineUser)
{
	Packet sendPacket;
	/* 通知在线用户更新排行榜 */
	sendPacket.type = PacketType::RENEW_RANKLIST;
	sprintf(sendPacket.data, "%s\n%d\n%d\n%d\n%d\n",
		onlineUser->username.c_str(), onlineUser->numberOfPokemens,
		onlineUser->rounds, onlineUser->wins, onlineUser->tops
	);
	this->m_onlineUserLocker.lock();
	for (const auto& perUser : this->m_onlineUsers)
	{
		if (perUser.second != nullptr)
		{
			_SendPacket_(perUser.second, sendPacket);
		}
	}
	this->m_onlineUserLocker.unlock();
}

/// <summary>
/// 获取排行榜
/// </summary>
/// <param name="onlineUser"></param>
void Server::_OnUpdateRanklistCallBack_(HOnlineUser onlineUser)
{
	/* 向该用户反馈排行榜信息 */
	char sendString[BUFLEN];
	sendString[0] = 0x0;
	int  userCount = 0;
	bool hasSend = false;
	Packet sendPacket;
	sendPacket.type = PacketType::SET_RANKLIST;
	this->m_rankedUserLocker.lock();
	for (const auto& perUser : this->m_rankedUsers)
	{ /* 将排行榜信息以最多10个为一组打包发送 */
		if (userCount == 10)
		{
			sprintf(sendPacket.data, "10\n%s", sendString);
			_SendPacket_(onlineUser, sendPacket);
			hasSend = true;
			sprintf(sendString,
				"%s,%d,%d,%d,%d\n",
				perUser->username.c_str(), perUser->numberOfPokemens,
				perUser->rounds, perUser->wins, perUser->tops
			);
		}
		else
		{
			sprintf(sendString + std::strlen(sendString),
				"%s,%d,%d,%d,%d\n",
				perUser->username.c_str(), perUser->numberOfPokemens,
				perUser->rounds, perUser->wins, perUser->tops
			);
		}
		++userCount;
	}
	this->m_rankedUserLocker.unlock();
	if (userCount > 0)
	{
		printf("%s\n", sendString);
		sprintf(sendPacket.data, "%d\n%s", userCount, sendString);
		_SendPacket_(onlineUser, sendPacket);
		hasSend = true;
	}
	if (!hasSend)
	{
		this->_RecvPacket_(onlineUser);
	}
}

void Server::_OnRemovePokemenCallBack_(HOnlineUser onlineUser, int removeId)
{
	Strings queryResult; // 查询结果
	char    szQuery[BUFLEN];
	Packet  sendPacket;

	onlineUser->m_needRemove = false;
	int pokemenId = removeId;
	if (removeId == 0)
	{
		pokemenId = onlineUser->m_pokemens.front().GetId();
	}

	sprintf(szQuery, REMOVE_POKEMEN_QUERY, pokemenId);
	this->m_hDatabase->Delete(szQuery);

	--onlineUser->numberOfPokemens;
	bool decOfTops = false;
	onlineUser->m_pokemens.remove_if([&pokemenId, &decOfTops](const Pokemen::Pokemen& pokemen) {
		if (pokemen.GetId() == pokemenId)
		{
			if (pokemen.GetLevel() == 15)
				decOfTops = true;
			return true;
		}
		return false;
	});
	if (decOfTops == true)
	{ /* 减少一个高阶精灵 */
		--onlineUser->tops;
	}

	if (onlineUser->numberOfPokemens == 0)
	{
		/* 为用户新发放一个小精灵 */
		Pokemen::Pokemen pokemen{ PokemenType::DEFAULT, 0x1 };
		sprintf(szQuery, INSERT_POKEMEN_QUERYSTRING,
			onlineUser->username.c_str(),
			(int)pokemen.GetType(), pokemen.GetName().c_str(),
			pokemen.GetHpoints(), pokemen.GetAttack(),
			pokemen.GetDefense(), pokemen.GetAgility(),
			pokemen.GetInterval(), pokemen.GetCritical(),
			pokemen.GetHitratio(), pokemen.GetParryratio(),
			pokemen.GetCareer(),
			pokemen.GetExp(), pokemen.GetLevel()
		);
		this->m_hDatabase->Insert(szQuery);

		onlineUser->numberOfPokemens = 1;
		sprintf(szQuery, "update users set numberOfPokemens=%d where name='%s'",
			onlineUser->numberOfPokemens, onlineUser->username.c_str()
		);
		this->m_hDatabase->Update(szQuery);

		if (removeId != 0)
		{
			/* 反馈该用户所拥有的小精灵 */
			sendPacket.type = PacketType::UPDATE_POKEMENS;
			sprintf(szQuery, SELECT_POKEMEN_BY_USER_QUERYSTRING, onlineUser->username.c_str());
			queryResult = this->m_hDatabase->Select(szQuery, NUMBER_OF_POKEMEN_COLUMNS);
			for (const auto& pokemen : queryResult)
			{
				onlineUser->InsertAPokemen(pokemen);
				sprintf(sendPacket.data, pokemen.c_str());
				_SendPacket_(onlineUser, sendPacket);
			}
		}
	}

	/* 更新用户列表数据 */
	this->m_rankedUserLocker.lock();
	RankedUsers::iterator rank = std::find_if(this->m_rankedUsers.begin(), this->m_rankedUsers.end(), [&onlineUser](HUser perUser) {
		return perUser->username == onlineUser->username;
	});
	(*rank)->numberOfPokemens = onlineUser->numberOfPokemens;
	(*rank)->numberOfPokemens = onlineUser->tops;
	this->m_rankedUserLocker.unlock();

	/* 向其他所有在线用户发送更新排行榜的通知 */
	this->_OnRenewRanklistCallBack_(onlineUser);
}

/// <summary>
/// 投递出境数据包
/// </summary>
/// <param name="onlineUser"></param>
/// <param name="sendPacket"></param>
/// <returns></returns>
bool Server::_SendPacket_(HOnlineUser onlineUser, const Packet& sendPacket)
{
	if (onlineUser == nullptr)
		return false;

	try
	{
		LPPER_IO_OPERATION_DATA sendIO = new PER_IO_OPERATION_DATA();
		onlineUser->IncIOSendCounter();

		printf("发送数据包类型=%d\n", (int)sendPacket.type);

		DWORD sendBytes;
		ZeroMemory(&(sendIO->overlapped), sizeof(OVERLAPPED));
		std::memcpy((LPCH)sendIO->buffer, (LPCH)&sendPacket, sizeof(Packet));
		sendIO->dataBuf.buf = sendIO->buffer;
		sendIO->dataBuf.len = sizeof(Packet);
		sendIO->opType = OPERATION_TYPE::SEND_POSTED;
		sendIO->sendBytes = 0;
		sendIO->totalBytes = sizeof(Packet);

		if (WSASend(onlineUser->m_client, &(sendIO->dataBuf), 1, &sendBytes, 0,
			&(sendIO->overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
				throw std::exception("网络异常。\n");
		}
		return true;
	}
	catch (const std::exception& e)
	{
		fprintf(stdout, "%s异常码：%d\n", e.what(), WSAGetLastError());
		return false;
	}
}

/// <summary>
/// 接收入境数据包
/// </summary>
/// <param name="onlineUser"></param>
/// <returns></returns>
bool Server::_RecvPacket_(HOnlineUser onlineUser)
{
	if (onlineUser == nullptr)
		return false;

	try
	{
		/* 可复用IO资源 */
		LPPER_IO_OPERATION_DATA recvIO = new PER_IO_OPERATION_DATA();
		onlineUser->IncIORecvCounter();

		ZeroMemory(&(recvIO->overlapped), sizeof(OVERLAPPED));
		recvIO->dataBuf.len = DATA_BUFSIZE;
		recvIO->dataBuf.buf = recvIO->buffer;
		recvIO->opType = OPERATION_TYPE::RECV_POSTED;
		DWORD flags = 0;
		DWORD recvBytes;
		if (WSARecv(onlineUser->m_client, &(recvIO->dataBuf), 1, &recvBytes, &flags,
			&(recvIO->overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				this->_OnConnectionLostCallBack_(nullptr, recvIO);
				throw std::exception("网络异常。\n");
			}
		}
		return true;
	}
	catch (const std::exception& e)
	{
		fprintf(stdout, "%s异常码：%d\n", e.what(), WSAGetLastError());
		return false;
	}
}

/// <summary>
/// 工作线程
/// </summary>
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
				this->_OnConnectionLostCallBack_(perClient, perIO);
			}
		}
		else
		{
			// 如果数据传送完了，则退出（close）
			if (bytesTransferred == 0
				&& (perIO->opType == OPERATION_TYPE::RECV_POSTED
					|| perIO->opType == OPERATION_TYPE::SEND_POSTED))
			{
				this->_OnConnectionLostCallBack_(perClient, perIO);
			}
			else if (perIO != nullptr && perClient != nullptr)
			{
				unsigned long userId = perClient->addr.sin_addr.S_un.S_addr;
				switch (perIO->opType)
				{
				case OPERATION_TYPE::RECV_POSTED:
				{
#ifdef DEBUG
					this->m_onlineUserLocker.lock();
					HOnlineUser ou = this->m_onlineUsers[userId];
					this->m_onlineUserLocker.unlock();
					if (ou != nullptr)
					{
						printf("成功接收入境数据包 RIO=%d SIO=%d IP=%s\n",
							ou->ReadIORecvCounter(),
							ou->ReadIOSendCounter(),
							inet_ntoa(perClient->addr.sin_addr));
					}
#endif // DEBUG

					perIO->buffer[bytesTransferred] = 0x0;
					std::memcpy((LPCH)&recvPacket, (LPCH)perIO->buffer, sizeof(Packet));

					this->m_onlineUserLocker.lock();
					HOnlineUser onlineUser = this->m_onlineUsers[userId];
					this->m_onlineUserLocker.unlock();
					if (onlineUser != nullptr)
					{
						int recvCounter = onlineUser->ReadIORecvCounter();

						if (_AnalyzePacket_(perClient, recvPacket) 
							|| recvCounter > 3)
						{
							this->m_onlineUserLocker.lock();
							HOnlineUser onlineUser = this->m_onlineUsers[userId];
							this->m_onlineUserLocker.unlock();
							if (onlineUser != nullptr)
							{
								onlineUser->DecIORecvCounter();
							}

							/* 释放掉该IO资源 */
							delete perIO;
							perIO = nullptr;
						}
						else
						{
							/* 预留该资源用于接收客户端的请求 */
							ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
							perIO->dataBuf.len = DATA_BUFSIZE;
							perIO->dataBuf.buf = perIO->buffer;
							perIO->opType = OPERATION_TYPE::RECV_POSTED;
							flags = 0;

							if (WSARecv(perClient->client, &(perIO->dataBuf), 1, &recvBytes, &flags,
								&(perIO->overlapped), NULL) == SOCKET_ERROR)
							{
								if (WSAGetLastError() != ERROR_IO_PENDING
									&& perClient != nullptr)
								{
									this->_OnConnectionLostCallBack_(perClient, perIO);
								}
							}
						}
					}
					else
					{
						/* 释放掉该IO资源 */
						delete perIO;
						perIO = nullptr;
					}
				}
				break;

				case OPERATION_TYPE::SEND_POSTED:
				{
					perIO->sendBytes += bytesTransferred;
					if (perIO->sendBytes < perIO->totalBytes)
					{
						/* 数据未发送完毕 继续投递 */
						ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
						perIO->dataBuf.buf = perIO->buffer + bytesTransferred;
						perIO->dataBuf.len = perIO->totalBytes - perIO->sendBytes;
						perIO->opType = OPERATION_TYPE::SEND_POSTED;

						if (WSASend(perClient->client, &(perIO->dataBuf), 1, &sendBytes, 0,
							&(perIO->overlapped), NULL) == SOCKET_ERROR)
						{
							if (WSAGetLastError() != ERROR_IO_PENDING
								&& perClient != nullptr)
							{
								this->_OnConnectionLostCallBack_(perClient, perIO);
							}
						}
					}
					else
					{
#ifdef DEBUG
						this->m_onlineUserLocker.lock();
						HOnlineUser ou = this->m_onlineUsers[userId];
						this->m_onlineUserLocker.unlock();
						if (ou != nullptr)
						{
							printf("成功投递出境数据包 RIO=%d SIO=%d IP=%s\n",
								ou->ReadIORecvCounter(),
								ou->ReadIOSendCounter(),
								inet_ntoa(perClient->addr.sin_addr));
						}
#endif // DEBUG
						this->m_onlineUserLocker.lock();
						HOnlineUser onlineUser = this->m_onlineUsers[userId];
						this->m_onlineUserLocker.unlock();
						if (onlineUser != nullptr)
						{
							onlineUser->DecIOSendCounter();
							int recvCounter = onlineUser->ReadIORecvCounter();
							if (recvCounter < 3)
							{
								onlineUser->IncIORecvCounter();
								/* 预留该资源用于接收客户端的请求 */
								ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
								perIO->dataBuf.len = DATA_BUFSIZE;
								perIO->dataBuf.buf = perIO->buffer;
								perIO->opType = OPERATION_TYPE::RECV_POSTED;
								flags = 0;

								if (WSARecv(perClient->client, &(perIO->dataBuf), 1, &recvBytes, &flags,
									&(perIO->overlapped), NULL) == SOCKET_ERROR)
								{
									if (WSAGetLastError() != ERROR_IO_PENDING
										&& perClient != nullptr)
									{
										this->_OnConnectionLostCallBack_(perClient, perIO);
									}
								}
							}
							else
							{
								/* 释放掉该IO资源 */
								delete perIO;
								perIO = nullptr;
							}
						}
						else
						{
							/* 释放掉该IO资源 */
							delete perIO;
							perIO = nullptr;
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

/// <summary>
/// 接受连接线程
/// </summary>
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

			this->m_onlineUserLocker.lock();
			if (this->m_onlineUsers[clientAddr.sin_addr.S_un.S_addr]
				!= nullptr)
			{
				delete m_onlineUsers[clientAddr.sin_addr.S_un.S_addr];
			}
			this->m_onlineUsers[clientAddr.sin_addr.S_un.S_addr]
				= new OnlineUser{ client, clientAddr };
			this->m_onlineUserLocker.unlock();

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
			WSARecv(client, &(perIO->dataBuf), 1, &recvBytes, &flags,
				&(perIO->overlapped), NULL);
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
		this->m_onlineUsers.clear();
		this->m_isServerOn = false;
	}
}
