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
	m_onlineUsers(), m_onlineUserLocker(),
	m_rankedUsers(), m_rankedUserLocker(),
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

	/* ���������û���¼ */
	_LoadAllUsers_();

	if (!_InitNetwork_())
		return INIT_NETWORK_ERROR;
	return INIT_SUCCESS;
}

bool Server::Run()
{
	if (this->m_isServerOn)
		return false;

	/* ���������߳� */
	try
	{
		// �����µ���ɶ˿�
		if ((this->m_completionPort = CreateIoCompletionPort(
			INVALID_HANDLE_VALUE, NULL, 0, 0)
			) == NULL)
		{
			WSACleanup();
			throw std::exception("��Դ���㣬�޷�������ɶ˿ڡ�\n");
		}

		// ��������Socket
		if ((this->m_serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
			WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
		{
			WSACleanup();
			throw std::exception("��Դ���㣬�޷�����Socket��\n");
		}

		m_serverAddr.sin_family = AF_INET;
		m_serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		m_serverAddr.sin_port = htons(PORT);

		if (bind(m_serverSocket, (LPSOCKADDR)&m_serverAddr,
			sizeof(SOCKADDR)) == SOCKET_ERROR)
		{
			closesocket(m_serverSocket);
			WSACleanup();
			throw std::exception("�޷��󶨵����ض˿ڡ�\n");
		}

		if (listen(m_serverSocket, 0) == SOCKET_ERROR)
		{
			closesocket(m_serverSocket);
			WSACleanup();
			throw std::exception("�����˿�ʧ�ܡ�\n");
		}

		this->m_isServerOn = true;
		SYSTEM_INFO SystemInfo; // ϵͳ��Ϣ
		GetSystemInfo(&SystemInfo);
		// ����CPU���������߳�
		for (int i = 0; i < (int)SystemInfo.dwNumberOfProcessors * 2; ++i)
		{ // �����̣߳�����ServerWorkerThread
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
	for (const auto& user : this->m_onlineUsers)
	{
		++id;
		SockaddrIn addr;
		addr.sin_addr.S_un.S_addr = user.first;
		sprintf(querySingle, "�û�%d: IP=%s\n", id, inet_ntoa(addr.sin_addr));
		queryResult += querySingle;
	}

	if (queryResult.size() == 0)
	{
		queryResult = "���û����ӡ�\n";
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

		// ��ʼ��Windows Socket����
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			throw std::exception("��ʼ��Windows Socket����ʧ�ܡ�\n");
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
void Server::_LoadAllUsers_()
{
	char queryString[BUFSIZ];
	sprintf(queryString, LOAD_ALL_USERS_QUERY);

	Strings queryResult = this->m_hDatabase->Select(queryString, 4);
	this->m_rankedUserLocker.lock();
	for (const auto& userInfos : queryResult)
	{
		Strings elems = SplitData(userInfos.c_str(), '\n');
		if (std::atoi(elems[1].c_str()) > 0)
		{ /* ֻ����ӵ��С������û� */
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

bool Server::_AnalyzePacket_(LPPER_HANDLE_DATA client, const Packet& recv)
{
	printf("���ݰ�����%d\n", (int)recv.type);
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

	default:
		return false;
	}
}

#define SELECT_USER_QUERYSTRING "select numberOfPokemens,rounds,wins,tops from users where name='%s' and password='%s'"
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
const static int initNumberOfPokemens = 3;
void Server::_DealWithLogin_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult;
	char    szQuery[BUFLEN];

	Strings userInfos = SplitData(data);
	Packet  sendPacket;

	printf("%s %s\n", inet_ntoa(client->addr.sin_addr), userInfos[0].c_str());

	int userId = client->addr.sin_addr.S_un.S_addr;
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
			// ������½��Ϣ
			Strings queryElems;
			if (!onlineUser->username.empty() || queryResult.empty())
				sendPacket.type = PacketType::LOGIN_FAILED;
			else
			{
				queryElems = SplitData(queryResult[0].c_str());
				sendPacket.type = PacketType::LOGIN_SUCCESS;
				onlineUser->SetUsername(userInfos[0]);
				sprintf(sendPacket.data, "%s", queryElems[0].c_str());
			}
			_SendPacket_(onlineUser, sendPacket);

			// ���������û��Լ�С������Ϣ
			if (sendPacket.type == PacketType::LOGIN_SUCCESS)
			{
				_OnLoginSuccessCallBack(onlineUser, userInfos, queryElems);
			}
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
	}
}

void Server::_OnLoginSuccessCallBack(HOnlineUser onlineUser, const Strings& userInfos, const Strings& queryElems)
{
	Strings queryResult;
	char    szQuery[BUFLEN];
	Packet  sendPacket;

	onlineUser->username = userInfos[0];
	onlineUser->numberOfPokemens = std::atoi(queryElems[0].c_str());
	onlineUser->rounds = std::atoi(queryElems[1].c_str());
	onlineUser->wins = std::atoi(queryElems[2].c_str());
	onlineUser->tops = std::atoi(queryElems[3].c_str());

	printf("��½�ɹ� �û���%s ������Ŀ��%d ����������%d ��ʤ������%d �߽׾��飺%d\n",
		onlineUser->username.c_str(), onlineUser->numberOfPokemens, onlineUser->rounds, onlineUser->wins, onlineUser->tops);

	if (onlineUser->numberOfPokemens == 0)
	{  /* �û���һ�ε�½�����������ֻС���� */
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

		/* �����û��������а� */
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
		/* ˢ�����а� */
		this->m_rankedUserLocker.lock();
		RankedUsers::iterator currentUser = std::find_if(this->m_rankedUsers.begin(), this->m_rankedUsers.end(),
			[&onlineUser](const HUser perUser) {
			return !perUser->username.compare(onlineUser->username);
		});
		this->m_rankedUserLocker.unlock();
		(*currentUser)->numberOfPokemens = onlineUser->numberOfPokemens;
		(*currentUser)->rounds = onlineUser->rounds;
		(*currentUser)->wins = onlineUser->wins;
		(*currentUser)->tops = onlineUser->tops;
	}
	this->_OnUpdatePokemensCallBack_(onlineUser);

	/* ����û��������а���Ϣ */
	char sendString[BUFLEN];
	sendString[0] = 0x0;
	int  userCount = 0;

	sendPacket.type = PacketType::SET_RANKLIST;
	this->m_rankedUserLocker.lock();
	for (const auto& perUser : this->m_rankedUsers)
	{ /* �����а���Ϣ�����10��Ϊһ�������� */
		if (userCount == 10)
		{
			sprintf(sendPacket.data, "10\n%s", sendString);
			_SendPacket_(onlineUser, sendPacket);
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
	}

	/* �����������û��������û�������״̬ */
	sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
	sprintf(sendPacket.data, "%s\nON\n", userInfos[0].c_str());
	this->m_onlineUserLocker.lock();
	for (const auto& otherUser : this->m_onlineUsers)
	{
		if (otherUser.first != onlineUser->GetUserID())
		{
			_SendPacket_(otherUser.second, sendPacket);
		}
	}
	this->m_onlineUserLocker.unlock();
}

void Server::_DealWithLogon_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult;
	char    szQuery[BUFLEN];

	Strings userInfos = SplitData(data);
	Packet  sendPacket;

	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser user = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();
		if (user != nullptr)
		{
			sprintf(szQuery,
				"insert into users values('%s','%s',0, 0, 0, 0)",
				userInfos[0].c_str(), userInfos[1].c_str()
			);

			if (this->m_hDatabase->Insert(szQuery))
				sendPacket.type = PacketType::LOGON_SUCCESS;
			else
				sendPacket.type = PacketType::LOGON_FAILED;
			_SendPacket_(user, sendPacket);
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
	}
}

void Server::_DealWithLogout_(LPPER_HANDLE_DATA client)
{
	Strings queryResult;
	Packet sendPacket;
	try
	{
		this->_OnConnectionLostCallBack_(client, nullptr);
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
	}
}

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
			char szUserNames[BUFLEN];
			szUserNames[0] = 0x0;
			int  cnt = 0;
			sendPacket.type = PacketType::SET_ONLINE_USERS;
			for (const auto& otherUser : this->m_onlineUsers)
			{ /* �����20���û���Ϊһ�������� */
				if (otherUser.first != userId)
				{
					++cnt;
					sprintf(szUserNames + std::strlen(szUserNames),
						"%s\n", otherUser.second->GetUsername().c_str()
					);
					if (cnt == 20)
					{
						sprintf(sendPacket.data, "20\n%s", szUserNames);
						_SendPacket_(onlineUser, sendPacket);

						cnt = 0;
						szUserNames[0] = 0x0;
					}
				}
			}
			if (cnt > 0)
			{
				sprintf(sendPacket.data, "%d\n%s", cnt, szUserNames);
				_SendPacket_(onlineUser, sendPacket);
			}
		}
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(e.what());
	}
}

void Server::_DealWithPVEResult_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult;
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
			++onlineUser->rounds;
			if (infos[0].compare("F") == 0)
				++onlineUser->wins;

			/* �����Ӧ�ľ��� */
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
			{ /* ����15����������� */
				pokemen->RenewProperty(prop, std::atoi(infos[14].c_str()));
				sprintf(szQuery,
					UPDATE_POKEMEN_QUERYSTRING,
					prop.m_hpoints, prop.m_attack, prop.m_defense, prop.m_agility,
					prop.m_interval, prop.m_critical, prop.m_hitratio, prop.m_parryratio,
					std::atoi(infos[14].c_str()), prop.m_exp, prop.m_level,
					prop.m_id
				);
				this->m_hDatabase->Update(szQuery);

				/* �ش����º�С���� */
				sendPacket.type = PacketType::UPDATE_POKEMENS;
				sprintf(sendPacket.data,
					POKEMEN_ALL_PROPERTIES,
					pokemen->GetId(), (int)pokemen->GetType(), pokemen->GetName().c_str(),
					pokemen->GetHpoints(), pokemen->GetAttack(), pokemen->GetDefense(), pokemen->GetAgility(),
					pokemen->GetInterval(), pokemen->GetCritical(), pokemen->GetHitratio(), pokemen->GetParryratio(),
					pokemen->GetCareer(), pokemen->GetExp(), pokemen->GetLevel()
				);
				_SendPacket_(onlineUser, sendPacket);

				/* ����߽׾������Ŀ */
				if (pokemen->GetLevel() == 15)
				{
					++onlineUser->tops;
				}
			}

			/* �������ݿ� */
			sprintf(szQuery,
				"update users set rounds=%d,wins=%d,tops=%d where name='%s'",
				onlineUser->rounds, onlineUser->wins, onlineUser->tops, onlineUser->username.c_str());
			this->m_hDatabase->Update(szQuery);

			/* �������а��е����� */
			this->m_rankedUserLocker.lock();
			RankedUsers::iterator rankedUser = std::find_if(this->m_rankedUsers.begin(), this->m_rankedUsers.end(),
				[&onlineUser](HUser perUser) {
				return perUser->username.compare(onlineUser->username);
			});
			(*rankedUser)->rounds = onlineUser->rounds;
			(*rankedUser)->wins = onlineUser->wins;
			(*rankedUser)->tops = onlineUser->tops;
			this->m_rankedUserLocker.unlock();

			/* Ϊ���������û��������а� */
			this->_OnRenewRanklistCallBack_(onlineUser);
		}
	}
	catch (const std::exception&)
	{
	}
}

void Server::_DealWithPromotePokemen_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult;
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
				pokemen->Promote(careerType);
				sprintf(szQuery,
					UPDATE_POKEMEN_QUERYSTRING,
					pokemen->GetHpoints(), pokemen->GetAttack(), pokemen->GetDefense(), pokemen->GetAgility(),
					pokemen->GetInterval(), pokemen->GetCritical(), pokemen->GetHitratio(), pokemen->GetParryratio(),
					pokemen->GetCareer(), pokemen->GetExp(), pokemen->GetLevel(),
					pokemenId
				);
				this->m_hDatabase->Update(szQuery);

				/* �ش����º�С���� */
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
		}
	}
	catch (const std::exception&)
	{
	}
}

void Server::_DealWithAddPokemen_(LPPER_HANDLE_DATA client)
{
	Strings queryResult;
	char    szQuery[BUFLEN];
	Packet  sendPacket;
	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();

		onlineUser->m_needRemove = false;

		/* Ϊ�û��·���һ��С���� */
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

		/* �������û������һ��С���� */
		sendPacket.type = PacketType::ADD_POKEMEN;
		sprintf(szQuery, SELECT_POKEMEN_BY_USER_QUERYSTRING, onlineUser->username.c_str());
		queryResult = this->m_hDatabase->Select(szQuery, NUMBER_OF_POKEMEN_COLUMNS);
		int lastPokemen = (int)queryResult.size() - 1;

		onlineUser->InsertAPokemen(queryResult[lastPokemen]);
		sprintf(sendPacket.data, queryResult[lastPokemen].c_str());
		_SendPacket_(onlineUser, sendPacket);

		/* �����û��б��е����� */
		this->m_rankedUserLocker.lock();
		(*std::find_if(this->m_rankedUsers.begin(), this->m_rankedUsers.end(), [&onlineUser](HUser perUser) {
			return perUser->username.compare(onlineUser->username);
		}))->numberOfPokemens = onlineUser->numberOfPokemens;
		this->m_rankedUserLocker.unlock();

		/* ���������������û����͸������а��֪ͨ */
		this->_OnRenewRanklistCallBack_(onlineUser);
	}
	catch (std::exception&)
	{

	}
}

#define REMOVE_POKEMEN_QUERY "delete from pokemens where identity=%d"
void Server::_DealWithSubPokemen_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult;
	char    szQuery[BUFLEN];
	Packet  sendPacket;
	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();

		int removeId = 0;
		sscanf(data, "%d", &removeId);
		if (!onlineUser->m_needRemove && removeId == 0)
		{
			onlineUser->m_needRemove = true;
		}
		else if (removeId > 0)
		{
			onlineUser->m_needRemove = false;

			sprintf(szQuery, REMOVE_POKEMEN_QUERY, removeId);
			this->m_hDatabase->Delete(szQuery);

			--onlineUser->numberOfPokemens;
			bool decOfTops = false;
			onlineUser->m_pokemens.remove_if([&removeId, &decOfTops](const Pokemen::Pokemen& pokemen) {
				if (pokemen.GetId() == removeId)
				{
					if (pokemen.GetLevel() == 15)
						decOfTops = true;
					return true;
				}
				return false;
			});
			if (decOfTops == true)
			{ /* ����һ���߽׾��� */
				--onlineUser->tops;
			}

			if (onlineUser->numberOfPokemens == 0)
			{
				/* Ϊ�û��·���һ��С���� */
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

				/* �������û���ӵ�е�С���� */
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

			/* �����û��б����� */
			this->m_rankedUserLocker.lock();
			RankedUsers::iterator rank = std::find_if(this->m_rankedUsers.begin(), this->m_rankedUsers.end(), [&onlineUser](HUser perUser) {
				return perUser->username.compare(onlineUser->username);
			});
			(*rank)->numberOfPokemens = onlineUser->numberOfPokemens;
			(*rank)->numberOfPokemens = onlineUser->tops;
			this->m_rankedUserLocker.unlock();

			/* ���������������û����͸������а��֪ͨ */
			this->_OnRenewRanklistCallBack_(onlineUser);
		}
	}
	catch (std::exception&)
	{

	}
}

void Server::_DealWithGetPokemensByUser_(LPPER_HANDLE_DATA client, const char data[])
{
	Strings queryResult;
	char    szQuery[BUFLEN];
	Packet  sendPacket;
	int userId = client->addr.sin_addr.S_un.S_addr;
	try
	{
		this->m_onlineUserLocker.lock();
		HOnlineUser onlineUser = this->m_onlineUsers[userId];
		this->m_onlineUserLocker.unlock();

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
	catch (std::exception&)
	{

	}
}

void Server::_OnConnectionLostCallBack_(LPPER_HANDLE_DATA lostClient, LPPER_IO_OPERATION_DATA lostIO)
{
	int lostId = lostClient->addr.sin_addr.S_un.S_addr;
	Packet sendPacket;

	this->m_onlineUserLocker.lock();
	this->m_releaseLocker.lock();
	if (this->m_needRelease[lostClient->client])
	{
		HOnlineUser lostUser = this->m_onlineUsers[lostId];
		if (lostUser != nullptr)
		{
			printf("%s�Ͽ�����\n", inet_ntoa(lostClient->addr.sin_addr));

			sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
			sprintf(sendPacket.data, "%s\nOFF\n", lostUser->GetUsername().c_str());
			for (auto& otherUser : this->m_onlineUsers)
			{
				if (otherUser.first != lostId)
				{
					_SendPacket_(otherUser.second, sendPacket);
				}
			}
		}
		/* �ͷ����� */
		this->m_onlineUsers.erase(lostId);

		closesocket(lostClient->client);
		this->m_needRelease[lostClient->client] = false;
		delete lostClient;
		delete lostIO;
	}
	this->m_releaseLocker.unlock();
	this->m_onlineUserLocker.unlock();
}

void Server::_OnUpdatePokemensCallBack_(HOnlineUser onlineUser)
{
	Packet  sendPacket;
	char    szQuery[BUFSIZ];
	Strings queryResult;

	/* �������û���ӵ�е�С���� */
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

void Server::_OnRenewRanklistCallBack_(HOnlineUser onlineUser)
{
	Packet sendPacket;
	/* ֪ͨ�����û��������а� */
	sendPacket.type = PacketType::RENEW_RANKLIST;
	sprintf(sendPacket.data, "%s\n%d\n%d\n%d\n%d\n",
		onlineUser->username.c_str(), onlineUser->numberOfPokemens,
		onlineUser->rounds, onlineUser->wins, onlineUser->tops
	);
	this->m_onlineUserLocker.lock();
	for (const auto& perUser : this->m_onlineUsers)
	{
		_SendPacket_(perUser.second, sendPacket);
	}
	this->m_onlineUserLocker.unlock();
}

/* Ͷ�ݳ������ݰ� */
bool Server::_SendPacket_(HOnlineUser onlineUser, const Packet& sendPacket)
{
	try
	{
		/* �ɸ���IO��Դ */
		LPPER_IO_OPERATION_DATA sendIO = new PER_IO_OPERATION_DATA();
		onlineUser->IncIOCounter();

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
				throw std::exception("�����쳣��\n");
		}
		return true;
	}
	catch (const std::exception& e)
	{
		fprintf(stdout, "%s�쳣�룺%d\n", e.what(), WSAGetLastError());
		return false;
	}
}

void Server::_WorkerThread_()
{
	DWORD bytesTransferred; // ���ݴ�����ֽ���
	LPPER_HANDLE_DATA perClient; // Socket����ṹ��
	LPPER_IO_OPERATION_DATA perIO; // I/O�����ṹ��
	DWORD flags;
	DWORD recvBytes;
	DWORD sendBytes;

	Packet recvPacket;
	Packet sendPacket;

	while (!this->m_errorOccured && this->m_isServerOn)
	{
		perClient = nullptr;
		// �����ɶ˿ڵ�״̬
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
			// ������ݴ������ˣ����˳���close��
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
					this->m_onlineUserLocker.lock();
					this->m_onlineUsers[userId]->DecIOCounter();
					this->m_onlineUserLocker.unlock();

					perIO->buffer[bytesTransferred] = 0x0;
					std::memcpy((LPCH)&recvPacket, (LPCH)perIO->buffer, sizeof(Packet));
					if (_AnalyzePacket_(perClient, recvPacket))
					{
						/* �ͷŵ���IO��Դ */
						delete perIO;
						perIO = nullptr;
					}
					else
					{
						/* Ԥ������Դ���ڽ��տͻ��˵����� */
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
				break;

				case OPERATION_TYPE::SEND_POSTED:
				{
					perIO->sendBytes += bytesTransferred;
					if (perIO->sendBytes < perIO->totalBytes)
					{
						/* ����δ������� ����Ͷ�� */
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
						this->m_onlineUserLocker.lock();
						printf("IO��Ŀ = %d\n", this->m_onlineUsers[userId]->ReadIOCounter());
						if (this->m_onlineUsers[userId]->ReadIOCounter() <= 2)
						{
							/* Ԥ������Դ���ڽ��տͻ��˵����� */
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
							/* �ͷŵ���IO��Դ */
							this->m_onlineUsers[userId]->DecIOCounter();

							delete perIO;
							perIO = nullptr;
						}
						this->m_onlineUserLocker.unlock();
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
				throw std::exception("�����쳣���������ӶϿ���\n");
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

			// ���䲢����Socket����ṹ��
			if ((perClient = new PER_HANDLE_DATA{ }) == nullptr)
			{
				throw std::exception("�ڴ治�㡣\n");
			}
			perClient->client = client;
			perClient->addr = clientAddr;

			fprintf(stdout, "�յ�%s����������\n", inet_ntoa(perClient->addr.sin_addr));

			// ����ͻ��˽���ͨ�ŵ��׽���Accept����ɶ˿�CompletionPort�����
			if (CreateIoCompletionPort(
				(HANDLE)client, this->m_completionPort, (ULONG_PTR)perClient, 0
			) == NULL)
			{
				throw std::exception("�����쳣����ɶ˿ڰ�ʧ�ܡ�\n");
			}

			// ΪI/O�����ṹ������ڴ�ռ�
			//if ((perIO = (LPPER_IO_OPERATION_DATA)GlobalAlloc(
			//	GPTR, sizeof(PER_IO_OPERATION_DATA)
			//)) == NULL)
			if ((perIO = new PER_IO_OPERATION_DATA{ }) == nullptr)
			{
				throw std::exception("�ڴ治�㡣\n");
			}

			// ��ʼ��I/O�����ṹ��
			ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
			perIO->dataBuf.len = DATA_BUFSIZE;
			perIO->dataBuf.buf = perIO->buffer;
			perIO->opType = OPERATION_TYPE::RECV_POSTED;
			flags = 0;

			// �������ݣ��ŵ�PerIoData��
			// ��PerIoData��ͨ�������߳��е�ServerWorkerThread����ȡ��
			if (WSARecv(client, &(perIO->dataBuf), 1, &recvBytes, &flags,
				&(perIO->overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					// throw std::exception("�����쳣��Ͷ���뾳��ʧ�ܡ�\n");
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		fprintf(stdout, "%s\n", e.what());
		/* �������е�ͨ���߳� */
		std::for_each(this->m_workers.begin(), this->m_workers.end(),
			[](Thread& worker) {
			if (worker.joinable())
				worker.join();
		});
		this->m_onlineUsers.clear();
		this->m_isServerOn = false;
	}
}

//void Server::_BeatThread_()
//{
//	Packet sendPacket;
//	sendPacket.type = PacketType::INVALID;
//
//	std::vector<HOnlineUser> onlineUsers;
//	while (!this->m_errorOccured && this->m_isServerOn)
//	{
//		Sleep(3000);
//
//		this->m_onlineUserLocker.lock();
//		std::for_each(this->m_onlineUsers.begin(), this->m_onlineUsers.end(),
//			[&onlineUsers](const std::pair<ULONG, HOnlineUser>& perUser) {
//			onlineUsers.push_back(perUser.second);
//		});
//		this->m_onlineUserLocker.unlock();
//
//		for (auto& perUser : onlineUsers)
//		{
//			this->_SendPacket_(perUser, sendPacket);
//		}
//	}
//}
