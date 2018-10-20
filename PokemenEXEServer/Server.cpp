#include "stdafx.h"
#include "Database.h"
#include "PokemenManager.h"

#include <thread>

constexpr int PORT = 27893;

constexpr int INIT_SUCCESS = 0x00000000;
constexpr int INIT_DATABASE_ERROR = 0xFFFFFFFE;
constexpr int INIT_NETWORK_ERROR = 0xFFFFFFFF;

constexpr auto DATABASE_USER = "root";
constexpr auto DATABASE_PASSWORD = "19981031";
constexpr auto DATABASE_NAME = "pokemen_user_database";

Server::Server() :
	m_hDatabase{new Database{}},
	m_userList{}
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

	if (_init_network_())
		return INIT_NETWORK_ERROR;

	return INIT_SUCCESS;
}

int Server::Run()
{
	std::thread accept_thread{ std::bind(&Server::_accept_, this) };
	accept_thread.detach();

	m_recvEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	std::thread recv_thread{ std::bind(&Server::_server_recv_thread_, this) };
	recv_thread.detach();

	std::thread send_thread{ std::bind(&Server::_server_send_thread_, this) };
	send_thread.detach();

	return INIT_SUCCESS;
}

void Server::WriteMessage(const Message& message)
{
	m_recvMutex.lock();

	m_recvMessageQueue.push(message);
	SetEvent(m_recvEvent);

	m_recvMutex.unlock();
}

std::string Server::GetClients() const
{
	std::string queryResult;
	char querySingle[1024];
	int id = 0;
	for (USER_LIST::const_iterator it = m_userList.begin();
		it != m_userList.end(); ++it)
	{
		++id;
		SOCKADDR_IN addr;
		addr.sin_addr.S_un.S_addr = (*it)->GetID();
		sprintf(querySingle, "Client %d: ip=%s\n", id, inet_ntoa(addr.sin_addr));

		queryResult += querySingle;
	}

	if (queryResult.size() == 0)
	{
		queryResult = "No clients connected.";
	}
	return queryResult;
}

int Server::_init_network_()
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
	m_serverAddr.sin_addr.S_un.S_addr = inet_addr("10.201.6.227");
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

void Server::_deal_with_user_launch_(const Message& message)
{
	std::vector<std::string> queryResult;
	char szQuery[BUFLEN];

	sprintf(szQuery, "select password from user_launch_info where name='%s'",
		message.data.user_info.name);
	queryResult = m_hDatabase->Select(szQuery, 1);

	m_userListMutex.lock();
	USER_LIST::iterator it_user = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUSERMANAGER& hUserManager) {
		return hUserManager->GetID() == message.id;
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
			if (std::strcmp(szPassword, message.data.user_info.password) != 0) packet.type = Packet::Type::LAUNCH_FAILED, printf("登陆失败。\n");
			else
			{
				USER_LIST::iterator it = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUSERMANAGER& hUserManager) {
					return hUserManager->GetName() == message.data.user_info.name;
				});
				if (it != m_userList.end()) packet.type = Packet::Type::LAUNCH_FAILED, printf("登陆失败。\n");
				else
				{
					(*it_user)->SetName(message.data.user_info.name);
					packet.type = Packet::Type::LAUNCH_SUCCESS, printf("登陆成功。\n");
				}
			}

			(*it_user)->WritePacket(packet);

			// 后续反馈登陆成功后用户的相关信息
			if (packet.type == Packet::Type::LAUNCH_SUCCESS)
			{
				sprintf(szQuery, "select pokemen_property from user_pokemen_info where user_name='%s'",
					message.data.user_info.name);
				queryResult = m_hDatabase->Select(szQuery, 1);

				if (queryResult.size() == 0)
				{	// 用户没有小精灵
					printf("用户至注册后第一次登陆。分配小精灵。\n");
					packet.type = Packet::Type::DISTRIBUTE_POKEMENS;

					(*it_user)->WritePacket(packet);

					packet.type = Packet::Type::INSERT_A_POKEMEN;
					for (int i = 0; i < 3; ++i)
					{
						Pokemen::PokemenManager newPokemen(Pokemen::PokemenType::DEFAULT);
						sprintf(packet.data, "NAME=%s,TY=%d,HP=%d,AT=%d,DE=%d,AG=%d,BR=%d,CR=%d,HI=%d,PA=%d",
							newPokemen.GetName().c_str(),
							(int)newPokemen.GetType(),
							newPokemen.GetHpoints(), newPokemen.GetAttack(), newPokemen.GetDefense(), newPokemen.GetAgility(),
							newPokemen.GetBreak(), newPokemen.GetCritical(), newPokemen.GetHitratio(), newPokemen.GetParryratio()
						);

						sprintf(szQuery,
							"insert into user_pokemen_info(pokemen_property,user_name) values('%s','%s')",
							packet.data,
							message.data.user_info.name
						);
						if (m_hDatabase->Insert(szQuery))
							(*it_user)->WritePacket(packet);
					}
				}
				else
				{
					packet.type = Packet::Type::INSERT_A_POKEMEN;
					for (std::vector<std::string>::const_iterator it_result = queryResult.begin();
						it_result != queryResult.end(); ++it_result)
					{
						sprintf(packet.data, "%s\n", it_result->c_str());
						(*it_user)->WritePacket(packet);
					}
				}

				packet.type = Packet::Type::UPDATE_USERS;
				sprintf(packet.data, "NAME=%s,STATE=ON\n", (*it_user)->GetName().c_str());
				for (USER_LIST::const_iterator it = m_userList.begin();
					it != m_userList.end(); ++it)
				{
					if (it == it_user)
						continue;
					(*it)->WritePacket(packet);
				}
			}
		}
		else
		{
			printf("登陆失败。\n");

			packet.type = Packet::Type::LAUNCH_FAILED;
			(*it_user)->WritePacket(packet);
		}
	}
	m_userListMutex.unlock();
}

void Server::_deal_with_user_register_(const Message& message)
{
	std::vector<std::string> queryResult;
	char szQuery[BUFLEN];

	m_userListMutex.lock();
	USER_LIST::iterator it_user = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUSERMANAGER& hUserManager) {
		return hUserManager->GetID() == message.id;
	});
	if (it_user == m_userList.end()) printf("致命错误：查找用户实例失败！\n");
	else
	{
		sprintf(szQuery, "insert into user_launch_info values('%s','%s')",
			message.data.user_info.name, message.data.user_info.password);

		Packet packet;
		if (m_hDatabase->Insert(szQuery))
			packet.type = Packet::Type::REGISTER_SUCCESS, printf("注册成功。\n");
		else
			packet.type = Packet::Type::REGISTER_FAILED, printf("注册失败。\n");
		(*it_user)->WritePacket(packet);
	}
	m_userListMutex.unlock();
}

void Server::_deal_with_user_closed_(const Message& message)
{
	std::vector<std::string> queryResult;

	m_userListMutex.lock();
	USER_LIST::iterator it_user = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUSERMANAGER& hUserManager) {
		return hUserManager->GetID() == message.id;
	});
	if (it_user == m_userList.end()) printf("致命错误：查找用户实例失败！\n");
	else
	{
		Packet packet;
		packet.type = Packet::Type::UPDATE_USERS;
		sprintf(packet.data, "NAME=%s,STATE=OFF\n", (*it_user)->GetName().c_str());
		for (USER_LIST::const_iterator it = m_userList.begin();
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

void Server::_deal_with_get_online_users_(const Message& message)
{
	m_userListMutex.lock();
	USER_LIST::iterator it_user = std::find_if(m_userList.begin(), m_userList.end(), [&message](const HUSERMANAGER& hUserManager) {
		return hUserManager->GetID() == message.id;
	});
	if (it_user == m_userList.end()) printf("致命错误：查找用户实例失败！\n");
	else
	{
		Packet packet;
		char szUserNames[BUFLEN];
		packet.type = Packet::Type::SET_ONLINE_USERS;
		int cnt = 0;

		std::memset(szUserNames, 0x0, BUFLEN);
		for (USER_LIST::const_iterator it = m_userList.begin();
			it != m_userList.end(); ++it)
		{
			if ((*it)->GetID() == message.id) continue;

			++cnt;
			sprintf(szUserNames + std::strlen(szUserNames), "%s,", (*it)->GetName().c_str());
			if (cnt == 20)
			{
				sprintf(packet.data, "TOTAL=20,%s", szUserNames);
				(*it_user)->WritePacket(packet);

				cnt = 0;
				std::memset(szUserNames, 0x0, BUFLEN);
			}
		}
		sprintf(packet.data, "TOTAL=%d,%s", cnt, szUserNames);
		(*it_user)->WritePacket(packet);
	}
	m_userListMutex.unlock();
}

void Server::_accept_()
{
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(SOCKADDR_IN);
	while (true)
	{
		SOCKET connectSocket = accept(m_serverSocket, (LPSOCKADDR)&clientAddr,
			&clientAddrLen);
		if (connectSocket == INVALID_SOCKET)
		{
			closesocket(m_serverSocket);
			return;
		}

		m_userListMutex.lock();
		m_userList.push_back(new UserManager{ connectSocket, clientAddr, this });
		m_userListMutex.unlock();
	}
}

void Server::_server_recv_thread_()
{
	Message message;
	std::vector<std::string> queryResult;
	while (true)
	{
		WaitForSingleObject(m_recvEvent, 2000);
		m_recvMutex.lock();
		ResetEvent(m_recvEvent);

		bool recvValid = false;
		if (!m_recvMessageQueue.empty())
		{
			recvValid = true;
			message = m_recvMessageQueue.front();
			m_recvMessageQueue.pop();
		}

		m_recvMutex.unlock();

		if (recvValid)
		{
			switch (message.type)
			{
			case Message::Type::USER_LAUNCH:
			{
				_deal_with_user_launch_(message);
				//std::thread thread{ std::bind(&Server::_deal_with_user_launch_, this) };
				//thread.detach();
			}
			break;

			case Message::Type::USER_REGISTER:
			{
				_deal_with_user_register_(message);
				//std::thread thread{ std::bind(&Server::_deal_with_user_register_, this) };
				//thread.detach();
			}
			break;

			case Message::Type::USER_CLOSED:
			{
				_deal_with_user_closed_(message);
				//std::thread thread{ std::bind(&Server::_deal_with_user_closed_, this) };
				//thread.detach();
			}
			break;

			case Message::Type::GET_ONLINE_USERS:
			{
				_deal_with_get_online_users_(message);
			}
			break;

			default:
				break;
			}
		}
	}
}

void Server::_server_send_thread_()
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
		std::strncpy(data.user_info.name, other.data.user_info.name, USER_NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	case Type::USER_CLOSED:
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
		std::strncpy(data.user_info.name, other.data.user_info.name, USER_NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	case Type::USER_CLOSED:
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
		std::strncpy(data.user_info.name, other.data.user_info.name, USER_NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	case Type::USER_CLOSED:
		id = other.id;
		break;

	default:
		break;
	}
	return *this;
}

Server::Message& Server::Message::operator=(Message && other)
{
	type = other.type;
	switch (type)
	{
	case Type::USER_LAUNCH:
	case Type::USER_REGISTER:
		std::strncpy(data.user_info.name, other.data.user_info.name, USER_NAME_LENGTH);
		std::strncpy(data.user_info.password, other.data.user_info.password, PASSWORD_LENGTH);
		id = other.id;
		break;

	case Type::USER_CLOSED:
		id = other.id;
		break;

	default:
		break;
	}
	return *this;
}
