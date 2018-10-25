#include "stdafx.h"
#include "Database.h"

#include <thread>

typedef Packet::Type PacketType;

constexpr int  PORT = 27893;

constexpr int  INIT_SUCCESS        = 0x00000000;
constexpr int  INIT_DATABASE_ERROR = 0xFFFFFFFE;
constexpr int  INIT_NETWORK_ERROR  = 0xFFFFFFFF;

constexpr auto DATABASE_USER     = "root";
constexpr auto DATABASE_PASSWORD = "19981031";
constexpr auto DATABASE_NAME     = "pokemen_user_database";

Server::Server() :
	m_hDatabase{ new Database{} },
	m_serverSocket(), m_serverAddr(),
	m_recvMutex(), m_recvEvent(nullptr), m_recvMessages(),
	m_userList(),
	m_userListMutex()
{
}

Server::~Server()
{
	WSACleanup();
	delete m_hDatabase;
}

int Server::Init()
{
	if (m_hDatabase->Connect(DATABASE_USER, DATABASE_PASSWORD, DATABASE_NAME))
		return INIT_DATABASE_ERROR;

	if (_InitNetwork_())
		return INIT_NETWORK_ERROR;

	return INIT_SUCCESS;
}

int Server::Run()
{
	Thread acceptThread{ std::bind(&Server::_ServerAcceptThread_, this) };
	acceptThread.detach();

	m_recvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	Thread recvThread{ std::bind(&Server::_ServerRecvThread_, this) };
	recvThread.detach();

	Thread sendThread{ std::bind(&Server::_ServerSendThread_, this) };
	sendThread.detach();

	return INIT_SUCCESS;
}

void Server::WriteMessage(const Message& message)
{
	m_recvMutex.lock();

	m_recvMessages.push(message);
	SetEvent(m_recvEvent);

	m_recvMutex.unlock();
}

String Server::GetClients() const
{
	String queryResult;
	char   querySingle[BUFLEN];
	int    id = 0;
	for (UserList::const_iterator it = m_userList.begin();
		it != m_userList.end(); ++it)
	{
		++id;
		SOCKADDR_IN addr;
		addr.sin_addr.S_un.S_addr = (*it)->GetUserID();
		sprintf(querySingle, "Client %d: ip=%s\n", id, inet_ntoa(addr.sin_addr));

		queryResult += querySingle;
	}

	if (queryResult.size() == 0)
	{
		queryResult = "No clients connected.";
	}
	return queryResult;
}

int Server::_InitNetwork_()
{
	int iRetVal = 0;
	WSADATA wsaData;

	iRetVal = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRetVal != 0)
		return iRetVal;

	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_serverSocket == INVALID_SOCKET)
	{
		WSACleanup();
		return -1;
	}

	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.S_un.S_addr = inet_addr("10.201.6.239");
	m_serverAddr.sin_port = htons(PORT);

	iRetVal = bind(m_serverSocket, (LPSOCKADDR)&m_serverAddr,
		sizeof(SOCKADDR));
	if (iRetVal == SOCKET_ERROR)
	{
		closesocket(m_serverSocket);
		WSACleanup();
		return -1;
	}

	iRetVal = listen(m_serverSocket, 0);
	if (iRetVal == SOCKET_ERROR)
	{
		closesocket(m_serverSocket);
		WSACleanup();
		return -1;
	}

	return 0;
}

void Server::_DealWithUserLaunch_(const Message& message)
{
	Strings queryResult;
	char    szQuery[BUFLEN];

	sprintf(szQuery, "select password from user_launch_info where name='%s'",
		message.data.user.name);
	queryResult = m_hDatabase->Select(szQuery, 1);

	m_userListMutex.lock();
	UserList::iterator it_user = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUser& hUserManager) {
		return hUserManager->GetUserID() == message.id;
	}); // 查找对应的用户连接实例
	if (it_user == m_userList.end()) printf("致命错误：查找用户实例失败！\n");
	else
	{
		Packet packet;
		if (queryResult.size() > 0)
		{	// 用户名存在
			char szPassword[BUFSIZ];
			sscanf(queryResult[0].c_str(), "%s\n", szPassword);

			// 优先反馈登陆是否成功
			if (std::strcmp(szPassword, message.data.user.password) != 0) packet.type = PacketType::LAUNCH_FAILED, printf("登陆失败。\n");
			else
			{
				UserList::iterator it = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUser& hUserManager) {
					return hUserManager->GetUsername() == message.data.user.name;
				});
				if (it != m_userList.end()) packet.type = PacketType::LAUNCH_FAILED, printf("登陆失败。\n");
				else
				{
					(*it_user)->SetUsername(message.data.user.name);
					packet.type = PacketType::LAUNCH_SUCCESS, printf("登陆成功。\n");
				}
			}
			(*it_user)->WritePacket(packet);

			// 后续反馈登陆成功后用户的相关信息
			if (packet.type == PacketType::LAUNCH_SUCCESS)
			{
				sprintf(szQuery, "select identity,pokemen_property from user_pokemen_info where user_name='%s'",
					message.data.user.name);
				queryResult = m_hDatabase->Select(szQuery, 2);

				if (queryResult.size() == 0)
				{	// 用户没有小精灵
					printf("用户至注册后第一次登陆。分配小精灵。\n");
					packet.type = PacketType::DISTRIBUTE_POKEMENS;
					(*it_user)->WritePacket(packet);

					for (int i = 0; i < 3; ++i)
					{
						Pokemen::PokemenManager newPokemen(PokemenType::DEFAULT);
						sprintf(szQuery,
							"insert into user_pokemen_info(pokemen_property,user_name) values('NAME=%s,TY=%d,HP=%d,AT=%d,DE=%d,AG=%d,BR=%d,CR=%d,HI=%d,PA=%d,EX=%d,LE=%d','%s')",
							newPokemen.GetName().c_str(),
							(int)newPokemen.GetType(),
							newPokemen.GetHpoints(), newPokemen.GetAttack(), newPokemen.GetDefense(), newPokemen.GetAgility(),
							newPokemen.GetBreak(), newPokemen.GetCritical(), newPokemen.GetHitratio(), newPokemen.GetParryratio(),
							newPokemen.GetExp(), newPokemen.GetLevel(),
							message.data.user.name
						);
						m_hDatabase->Insert(szQuery);
					}
					// 重新查询小精灵情况 获取ID值
					sprintf(szQuery, "select identity,pokemen_property from user_pokemen_info where user_name='%s'",
						message.data.user.name);
					queryResult = m_hDatabase->Select(szQuery, 2);
				}

				packet.type = PacketType::INSERT_A_POKEMEN;
				for (Strings::const_iterator it_result = queryResult.begin();
					it_result != queryResult.end(); ++it_result)
				{
					sprintf(packet.data, "%s", it_result->c_str());
					(*it_user)->InsertAPokemen(*it_result);
					(*it_user)->WritePacket(packet);
				}

				packet.type = PacketType::UPDATE_USERS;
				sprintf(packet.data, "NAME=%s,STATE=ON\n", (*it_user)->GetUsername().c_str());
				for (UserList::const_iterator it_other = m_userList.begin();
					it_other != m_userList.end(); ++it_other)
				{
					if (it_other == it_user)
						continue;
					(*it_other)->WritePacket(packet);
				}
			}
		}
		else
		{
			printf("登陆失败。\n");

			packet.type = PacketType::LAUNCH_FAILED;
			(*it_user)->WritePacket(packet);
		}
	}
	m_userListMutex.unlock();
}

void Server::_DealWithUserRegister_(const Message& message)
{
	Strings queryResult;
	char    szQuery[BUFLEN];

	m_userListMutex.lock();
	UserList::iterator it_user = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUser& hUserManager) {
		return hUserManager->GetUserID() == message.id;
	});
	if (it_user == m_userList.end()) printf("致命错误：查找用户实例失败！\n");
	else
	{
		sprintf(szQuery, "insert into user_launch_info values('%s','%s')",
			message.data.user.name, message.data.user.password);

		Packet packet;
		if (m_hDatabase->Insert(szQuery))
			packet.type = PacketType::REGISTER_SUCCESS, printf("注册成功。\n");
		else
			packet.type = PacketType::REGISTER_FAILED, printf("注册失败。\n");
		(*it_user)->WritePacket(packet);
	}
	m_userListMutex.unlock();
}

void Server::_DealWithUserClosed_(const Message& message)
{
	Strings queryResult;

	m_userListMutex.lock();
	UserList::iterator it_user = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUser& hUserManager) {
		return hUserManager->GetUserID() == message.id;
	});
	if (it_user == m_userList.end()) printf("致命错误：查找用户实例失败！\n");
	else
	{
		Packet packet;
		packet.type = PacketType::UPDATE_USERS;
		sprintf(packet.data, "NAME=%s,STATE=OFF\n", (*it_user)->GetUsername().c_str());
		for (UserList::const_iterator it = m_userList.begin();
			it != m_userList.end(); ++it)
		{
			if (it == it_user)
				continue;
			(*it)->WritePacket(packet);
		}

		delete *it_user;
		*it_user = nullptr;

		m_userList.erase(it_user);
	}
	m_userListMutex.unlock();
}

void Server::_DealWithGetOnlineUsers_(const Message& message)
{
	m_userListMutex.lock();
	UserList::iterator it_user = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUser& hUserManager) {
		return hUserManager->GetUserID() == message.id;
	});
	if (it_user == m_userList.end()) printf("致命错误：查找用户实例失败！\n");
	else
	{
		char   szUserNames[BUFLEN];
		int    cnt = 0;
		Packet packet;
		packet.type = PacketType::SET_ONLINE_USERS;

		ZeroMemory(szUserNames, sizeof(szUserNames));
		for (UserList::const_iterator it_other = m_userList.begin();
			it_other != m_userList.end(); ++it_other)
		{
			if ((*it_other)->GetUserID() == message.id) continue;

			++cnt;
			sprintf(szUserNames + std::strlen(szUserNames), "%s,", (*it_other)->GetUsername().c_str());
			if (cnt == 20)
			{
				sprintf(packet.data, "TOTAL=20,%s", szUserNames);
				(*it_user)->WritePacket(packet);

				cnt = 0;
				ZeroMemory(szUserNames, sizeof(szUserNames));
			}
		}
		sprintf(packet.data, "TOTAL=%d,%s", cnt, szUserNames);
		(*it_user)->WritePacket(packet);
	}
	m_userListMutex.unlock();
}

void Server::_DealWithBattleResult_(const Message& message)
{
}

void Server::_ServerAcceptThread_()
{
	SockaddrIn clientAddr;
	int clientAddrLen = sizeof(SockaddrIn);
	while (true)
	{
		Socket connectSocket = accept(m_serverSocket, (LPSOCKADDR)&clientAddr,
			&clientAddrLen);
		if (connectSocket == INVALID_SOCKET)
		{
			closesocket(m_serverSocket);
			return;
		}

		m_userListMutex.lock();
		m_userList.push_back(new UserManager{ connectSocket, clientAddr, *this });
		m_userListMutex.unlock();
	}
}

void Server::_ServerRecvThread_()
{
	Message message;
	Strings queryResult;
	while (true)
	{
		WaitForSingleObject(m_recvEvent, 2000);
		m_recvMutex.lock();
		ResetEvent(m_recvEvent);

		bool recvValid = false;
		if (!m_recvMessages.empty())
		{
			recvValid = true;
			message = m_recvMessages.front();
			m_recvMessages.pop();
		}

		m_recvMutex.unlock();

		if (recvValid)
		{
			switch (message.type)
			{
			case MessageType::USER_LAUNCH:
			{
				_DealWithUserLaunch_(message);
				//std::thread thread{ std::bind(&Server::_DealWithUserLaunch_, this) };
				//thread.detach();
			}
			break;

			case MessageType::USER_REGISTER:
			{
				_DealWithUserRegister_(message);
				//std::thread thread{ std::bind(&Server::_DealWithUserRegister_, this) };
				//thread.detach();
			}
			break;

			case MessageType::USER_CLOSED:
			{
				_DealWithUserClosed_(message);
				//std::thread thread{ std::bind(&Server::_DealWithUserClosed_, this) };
				//thread.detach();
			}
			break;

			case MessageType::GET_ONLINE_USERS:
			{
				_DealWithGetOnlineUsers_(message);
			}
			break;

			case MessageType::HANDLE_BATTLE_RESULT:
			{
				_DealWithBattleResult_(message);
			}
			break;

			default:
				break;
			}
		}
	}
}

void Server::_ServerSendThread_()
{
	while (true)
	{

	}
}

Server::Message::Message()
{
	std::memset(&data, 0x0, sizeof(data));
}

Server::Message::Message(const Message& other) :
	type(other.type)
{
	switch (type)
	{
	case Type::USER_LAUNCH:
	case Type::USER_REGISTER:
	case Type::HANDLE_BATTLE_RESULT:
		std::strncpy(data.user.name, other.data.user.name, USER_NAME_LENGTH);
		std::strncpy(data.user.password, other.data.user.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	case Type::USER_CLOSED:
	case Type::GET_ONLINE_USERS:
		id = other.id;
		break;

	default:
		break;
	}
}

Server::Message::Message(Message&& other) :
	type(other.type)
{
	switch (type)
	{
	case Type::USER_LAUNCH:
	case Type::USER_REGISTER:
	case Type::HANDLE_BATTLE_RESULT:
		std::strncpy(data.user.name, other.data.user.name, USER_NAME_LENGTH);
		std::strncpy(data.user.password, other.data.user.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	case Type::USER_CLOSED:
	case Type::GET_ONLINE_USERS:
		id = other.id;
		break;

	default:
		break;
	}
}

Server::Message& Server::Message::operator=(const Message& other)
{
	type = other.type;
	switch (type)
	{
	case Type::USER_LAUNCH:
	case Type::USER_REGISTER:
	case Type::HANDLE_BATTLE_RESULT:
		std::strncpy(data.user.name, other.data.user.name, USER_NAME_LENGTH);
		std::strncpy(data.user.password, other.data.user.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	case Type::USER_CLOSED:
	case Type::GET_ONLINE_USERS:
		id = other.id;
		break;

	default:
		break;
	}
	return *this;
}

Server::Message& Server::Message::operator=(Message&& other)
{
	type = other.type;
	switch (type)
	{
	case Type::USER_LAUNCH:
	case Type::USER_REGISTER:
	case Type::HANDLE_BATTLE_RESULT:
		std::strncpy(data.user.name, other.data.user.name, USER_NAME_LENGTH);
		std::strncpy(data.user.password, other.data.user.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	case Type::USER_CLOSED:
	case Type::GET_ONLINE_USERS:
		id = other.id;
		break;

	default:
		break;
	}
	return *this;
}
