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

static Strings SplitData(const char data[])
{
	String per;
	Strings ans;
	for (int pos = 0; pos < std::strlen(data); ++pos)
	{
		if (pos == '\n')
		{
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

Server::Server() :
	m_hDatabase{ new Database{} },
	m_serverSocket(), m_serverAddr(),
	m_users(), m_userLocker(),
	m_completionPort(nullptr), 
	m_acceptDriver(), m_workers(), 
	m_errorOccured(false),
	m_isServerOn(false)
{
}

Server::~Server()
{
	this->m_isServerOn = false;
	if (this->m_acceptDriver.joinable())
		this->m_acceptDriver.join();

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

	/* ���������߳� */
	try
	{
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
		this->m_acceptDriver
			= std::move(Thread{ std::bind(&Server::_ServerAcceptThread_, this) });
	}
	catch (std::exception& e)
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
		m_serverAddr.sin_addr.S_un.S_addr = inet_addr("10.201.6.248");
		m_serverAddr.sin_port = htons(PORT);

		if (bind(m_serverSocket, (LPSOCKADDR)&m_serverAddr,
			sizeof(SOCKADDR)) == SOCKET_ERROR)
		{
			closesocket(m_serverSocket);
			WSACleanup();
			throw std::exception("�޷��󶨵����ض˿ڡ�\n");
		}

		if (listen(m_serverSocket, 5) == SOCKET_ERROR)
		{
			closesocket(m_serverSocket);
			WSACleanup();
			throw std::exception("�����˿�ʧ�ܡ�\n");
		}
	}
	catch (const std::exception& e)
	{
		fprintf(stdout, "%s\n", e.what());
		return false;
	}
	return true;
}

bool Server::_AnalyzePacket_(SockaddrIn client, const Packet& recv)
{
	switch (recv.type)
	{
	case PacketType::LOGIN_REQUEST:
		_DealWithLogin_(client.sin_addr.S_un.S_addr, recv.data);
		break;

	case PacketType::LOGON_REQUEST:
		_DealWithLogon_(client.sin_addr.S_un.S_addr, recv.data);
		break;

	case PacketType::LOGOUT:
		_DealWithLogout_(client.sin_addr.S_un.S_addr);
		break;

	case PacketType::PVE_RESULT:
		_DealWithPVEResult_(client.sin_addr.S_un.S_addr, recv.data);
		break;

	default:
		break;
	}
	return false;
}

#define INSERT_POKEMEN_QUERYSTRING 	"\
insert into pokemens(user,type,name,\
hpoints,attack,defense,agility,break,critical,hitratio,parryratio,\
career,exp,level) values('%s',%d,'%s',\
%d,%d,%d,%d,%d,%d,%d,%d,\
%d,%d,%d)"

#define SELECT_POKEMEN_QUERYSTRING 	"\
select identity,type,name,\
hpoints,attack,defense,agility,break,critical,hitratio,parryratio,\
career,exp,level from pokemens where user='%s'"

void Server::_DealWithLogin_(ULONG identity, const char data[])
{
	Strings queryResult;
	char    szQuery[BUFLEN];

	Strings userInfos = SplitData(data);
	Packet  sendPacket;
	try
	{
		sprintf(szQuery, 
			"select numberOfPokemens from user_launch_info where name='%s',password='%s'",
			userInfos[0].c_str(), userInfos[1].c_str());
		queryResult = m_hDatabase->Select(szQuery, 1);

		this->m_userLocker.lock();
		HUser user = this->m_users[identity];
		this->m_userLocker.unlock();
		if (user != nullptr)
		{
			// ������½��Ϣ
			if (!user->m_username.empty() || queryResult.empty())
				sendPacket.type = PacketType::LOGIN_FAILED;
			else
			{
				sendPacket.type = PacketType::LOGIN_SUCCESS;
				user->SetUsername(userInfos[0]);
				sprintf(sendPacket.data, "%s\n", queryResult[0].c_str());
			}
			_SendPacket_(user, sendPacket);

			// ���������û��Լ�С������Ϣ
			if (sendPacket.type == PacketType::LOGIN_SUCCESS)
			{
				user->m_username = userInfos[0];

				int numberOfPokemens = std::atoi(queryResult[0].c_str());
				if (numberOfPokemens == 0)
				{
					/* �û���һ�ε�½�����������ֻС���� */
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

				/* �����ݿ�ȡ��С������Ϣ */
				sprintf(szQuery, SELECT_POKEMEN_QUERYSTRING, userInfos[0].c_str());
				queryResult = this->m_hDatabase->Select(szQuery, 14);
				for (const auto& pokemen : queryResult)
				{
					user->InsertAPokemen(pokemen);
					sprintf(sendPacket.data, pokemen.c_str());
					_SendPacket_(user, sendPacket);
				}

				sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
				sprintf(sendPacket.data, "%s\nON\n", userInfos[0].c_str());
				this->m_userLocker.lock();
				for (const auto& other : this->m_users)
				{
					if (other.first != identity)
					{
						_SendPacket_(other.second, sendPacket);
					}
				}
				this->m_userLocker.unlock();
			}
		}
	}
	catch (const std::exception& e)
	{
		
	}
}

void Server::_DealWithLogon_(ULONG identity, const char data[])
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
				"insert into user values('%s','%s',0)",
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
		
	}
}

void Server::_DealWithLogout_(ULONG identity)
{
	Strings queryResult;
	Packet sendPacket;
	try
	{
		this->m_userLocker.lock();
		HUser user = this->m_users[identity];
		this->m_userLocker.unlock();
		if (user != nullptr)
		{
			sendPacket.type = PacketType::UPDATE_ONLINE_USERS;
			sprintf(sendPacket.data, "%s\nOFF\n", user->GetUsername().c_str());
			for (auto& other : this->m_users)
			{
				if (other.first != identity)
				{
					_SendPacket_(other.second, sendPacket);
				}
			}
			this->m_users.erase(identity);
		}
	}
	catch (const std::exception&)
	{
		
	}
}

void Server::_DealWithGetOnlineUsers_(ULONG identity, const char data[])
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
						_SendPacket_(user, sendPacket);

						cnt = 0;
						ZeroMemory(szUserNames, sizeof(szUserNames));
					}
				}
			}
			if (cnt > 0)
			{
				sprintf(sendPacket.data, "%d\n%s", cnt, szUserNames);
				_SendPacket_(user, sendPacket);
			}
		}
	}
	catch (const std::exception& e)
	{

	}
}

void Server::_DealWithPVEResult_(ULONG identity, const char data[])
{
}

/* Ͷ�ݳ������ݰ� */
void Server::_SendPacket_(HUser user, const Packet& send)
{
	try
	{
		LPPER_IO_OPERATION_DATA perIO
			= (LPPER_IO_OPERATION_DATA)GlobalAlloc(
				GPTR, 
				sizeof(PER_IO_OPERATION_DATA)
			);
		if (perIO == NULL)
			throw std::exception("�ڴ治�㡣\n");

		/* ���÷��Ͷ��� */
		DWORD sendBytes;
		ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
		std::memcpy(perIO->buffer, (LPCH)&send, sizeof(Packet));
		perIO->dataBuf.buf = perIO->buffer;
		perIO->dataBuf.len = DATA_BUFSIZE;
		perIO->opType = OPERATION_TYPE::SEND_POSTED;
		perIO->sendBytes = 0;
		perIO->totalBytes = DATA_BUFSIZE;

		if (WSASend(user->m_client, &(perIO->dataBuf), 1, &sendBytes, 0,
			&(perIO->overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
				throw std::exception("�����쳣��\n");
		}
	}
	catch (const std::exception& e)
	{
		
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

	while (!this->m_errorOccured && this->m_isServerOn)
	{
		perClient = NULL;
		// �����ɶ˿ڵ�״̬
		if (GetQueuedCompletionStatus(this->m_completionPort, &bytesTransferred,
			(PULONG_PTR)&perClient, (LPOVERLAPPED*)&perIO, INFINITE) == 0)
		{
			if (perClient != NULL)
			{
				closesocket(perClient->client);
				GlobalFree(perClient);
				GlobalFree(perIO);
			}
		}
		else
		{
			// ������ݴ������ˣ����˳���close��
			if (bytesTransferred == 0
				&& (perIO->opType == OPERATION_TYPE::RECV_POSTED 
					|| perIO->opType == OPERATION_TYPE::SEND_POSTED))
			{
				closesocket(perClient->client);
				GlobalFree(perClient);
				GlobalFree(perIO);
				continue;
			}
			else if (perIO != NULL && perClient != NULL)
			{
				switch (perIO->opType)
				{
				case OPERATION_TYPE::RECV_POSTED:
				{
					perIO->buffer[bytesTransferred] = 0x0;
					std::memcpy((LPCH)&recvPacket, perIO->buffer, sizeof(Packet));
					_AnalyzePacket_(perClient->addr, recvPacket);

					// ��ʼ��I/O�����ṹ��
					ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
					perIO->dataBuf.len = DATA_BUFSIZE;
					perIO->dataBuf.buf = perIO->buffer;
					perIO->opType = OPERATION_TYPE::RECV_POSTED;
					flags = 0;

					// �������ݣ��ŵ�PerIoData��
					// ��PerIoData��ͨ�������߳��е�ServerWorkerThread����ȡ��
					if (WSARecv(perClient->client, &(perIO->dataBuf), 1, &recvBytes, &flags,
						&(perIO->overlapped), NULL) == SOCKET_ERROR)
					{
						if (WSAGetLastError() != ERROR_IO_PENDING
							&& perClient != NULL)
						{
							closesocket(perClient->client);
							GlobalFree(perClient);
							GlobalFree(perIO);
						}
					}

				}
				break;

				case OPERATION_TYPE::SEND_POSTED:
				{
					perIO->sendBytes += bytesTransferred;
					if (perIO->sendBytes <= perIO->totalBytes)
					{
						/* ����δ������� ����Ͷ�� */
						ZeroMemory(&(perIO->overlapped), sizeof(OVERLAPPED));
						perIO->dataBuf.buf = perIO->buffer + bytesTransferred;
						perIO->dataBuf.len = DATA_BUFSIZE - perIO->sendBytes;

						if (WSASend(perClient->client, &(perIO->dataBuf), 1, &sendBytes, 0,
							&(perIO->overlapped), NULL) == SOCKET_ERROR)
						{
							if (WSAGetLastError() != ERROR_IO_PENDING
								&& perClient != NULL)
							{
								closesocket(perClient->client);
								GlobalFree(perClient);
								GlobalFree(perIO);
							}
						}
					}
					else
					{
						GlobalFree(perClient);
						GlobalFree(perIO);
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

			this->m_userLocker.lock();
			if (this->m_users[clientAddr.sin_addr.S_un.S_addr]
				!= nullptr)
			{
				delete m_users[clientAddr.sin_addr.S_un.S_addr];
			}
			this->m_users[clientAddr.sin_addr.S_un.S_addr]
				= new User{ clientAddr };
			this->m_userLocker.unlock();

			// ���䲢����Socket����ṹ��
			if ((perClient = (LPPER_HANDLE_DATA)GlobalAlloc(
				GPTR, sizeof(PER_HANDLE_DATA)
			)) == NULL)
			{
				throw std::exception("�ڴ治�㡣\n");
			}
			perClient->client = client;
			perClient->addr = clientAddr;

			// ����ͻ��˽���ͨ�ŵ��׽���Accept����ɶ˿�CompletionPort�����
			if (CreateIoCompletionPort(
				(HANDLE)client, this->m_completionPort, (DWORD)perClient, 0
			) == NULL)
			{
				throw std::exception("�����쳣����ɶ˿ڰ�ʧ�ܡ�\n");
			}

			// ΪI/O�����ṹ������ڴ�ռ�
			if ((perIO = (LPPER_IO_OPERATION_DATA)GlobalAlloc(
				GPTR, sizeof(PER_IO_OPERATION_DATA)
			)) == NULL)
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
					throw std::exception("�����쳣��Ͷ���뾳��ʧ�ܡ�\n");
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
		this->m_users.clear();
		this->m_isServerOn = false;
	}
}