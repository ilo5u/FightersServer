#include "stdafx.h"

#include <thread>

UserManager::UserManager(const SOCKET& connectSocket, const SOCKADDR_IN& clientAddr, const HSERVER hServer) :
	m_connectSocket(connectSocket), m_clientAddr(clientAddr), m_hServer(hServer),
	m_readBattleMessageThread(nullptr)
{
	std::thread recvThread{ std::bind(&UserManager::_client_recv_thread_, this) };
	recvThread.detach();

	std::thread sendThread{ std::bind(&UserManager::_client_send_thread_, this) };
	sendThread.detach();

	m_sendEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_sendClosedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_recvClosedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

UserManager::~UserManager()
{
	// 关闭读取对战相关线程
	m_isReadBattleMessageClosed = true;
	if (m_readBattleMessageThread->joinable())
		m_readBattleMessageThread->join();
	delete m_readBattleMessageThread;
	m_readBattleMessageThread = nullptr;

	m_battle_stage.Clear();

	WaitForSingleObject(m_recvClosedEvent, INFINITE);
	WaitForSingleObject(m_sendClosedEvent, INFINITE);

	CloseHandle(m_recvClosedEvent);
	CloseHandle(m_sendClosedEvent);
	CloseHandle(m_sendEvent);

	closesocket(m_connectSocket);
}

void UserManager::InsertAPokemen(const std::string& info)
{
	int pos = info.find_first_of(',');
	std::string first{ info.substr(0, pos) };
	std::string second{ info.substr(pos + 1, info.size() - pos - 1) };

	int identity;
	char name[BUFSIZ];
	int type;
	int hpoints, attack, defense, agility;
	int brea, critical, hitratio, parryratio;
	int exp, level;
	sscanf(first.c_str(), "%d\nNAME=%s", &identity, name);
	sscanf(second.c_str(), "TY=%d,HP=%d,AT=%d,DE=%d,AG=%d,BR=%d,CR=%d,HI=%d,PA=%d,EX=%d,LE=%d",
		&type,
		&hpoints, &attack, &defense, &agility,
		&brea, &critical, &hitratio, &parryratio,
		&exp, &level);
	Pokemen::PokemenManager pokemen{ static_cast<Pokemen::PokemenType>(type) };
	pokemen.SetID(identity);
	pokemen.SetName(name);

	pokemen.SetHpoints(hpoints);
	pokemen.SetAttack(attack);
	pokemen.SetDefense(defense);
	pokemen.SetAgility(agility);
	pokemen.SetBreak(brea);

	pokemen.SetCritical(critical);
	pokemen.SetHitratio(hitratio);
	pokemen.SetParryratio(parryratio);

	pokemen.SetExp(exp);
	pokemen.SetLevel(level);

	m_pokemens.push_back(pokemen);
}

void UserManager::WritePacket(const Packet& packet)
{
	m_sendMutex.lock();

	m_sendPacketQueue.push(packet);
	SetEvent(m_sendEvent);

	m_sendMutex.unlock();
}

ULONG UserManager::GetID() const
{
	return m_clientAddr.sin_addr.S_un.S_addr;
}

void UserManager::SetName(const std::string& name)
{
	if (m_userName.empty())
		m_userName = name;
}

std::string UserManager::GetName() const
{
	return m_userName;
}

constexpr int DEFAULT_BUFFLEN = 4096;

void UserManager::_read_battle_message_()
{
	Packet packet;
	packet.type = Packet::Type::BATTLE_MESSAGE;

	std::string battleMessage;
	while (!m_isReadBattleMessageClosed)
	{
		battleMessage = std::move(m_battle_stage.ReadMessage().wsOptions.c_str());
		std::memcpy(packet.data, battleMessage.c_str(), battleMessage.size());
		packet.data[battleMessage.size()] = 0x0;

		if (std::strncmp(packet.data, "GAME END", std::strlen("GAME END")) == 0)
			break;

		WritePacket(packet);
	}

	Server::Message message;
	message.type = Server::Message::Type::HANDLE_BATTLE_RESULT;
	message.data.battle_result.battleType = m_battleType;
	sscanf(packet.data, "GAME END WITH %d", &message.data.battle_result.winner);
	message.data.battle_result.firstId = m_battle_stage.GetFirstPlayerId();
	message.data.battle_result.secondId = m_battle_stage.GetSecondPlayerId();
	message.id = m_clientAddr.sin_addr.S_un.S_addr;

	m_hServer->WriteMessage(message);
}

void UserManager::_client_recv_thread_()
{
	int iRecvBytes = 0;
	Packet::Type packetType;

	char szRecvBuf[BUFLEN];
	Server::Message message;
	while (!m_isClosed)
	{
		// Recive message from client
		std::memset(szRecvBuf, 0x0, sizeof(szRecvBuf));
		iRecvBytes = recv(m_connectSocket, szRecvBuf, sizeof(szRecvBuf), 0);
		
		message.id = m_clientAddr.sin_addr.S_un.S_addr;
		if (iRecvBytes > 0)
		{
			sscanf(szRecvBuf, "ID=%d\n", &packetType);
			switch (packetType)
			{
			case Packet::Type::LAUNCH_REQUEST:
			{
				if (m_userName.empty())
				{
					message.type = Server::Message::Type::USER_LAUNCH;
					sscanf(szRecvBuf, "ID=%d\nNAME=%s\nPASSWORD=%s\n", &packetType,
						message.data.user_info.name, message.data.user_info.password);

					printf("\n登陆请求： NAME=%s PASSWORD=%s\n\n",
						message.data.user_info.name, message.data.user_info.password);

					m_hServer->WriteMessage(message);
				}
			}
			break;

			case Packet::Type::REGISTER_REQUEST:
			{
				message.type = Server::Message::Type::USER_REGISTER;
				sscanf(szRecvBuf, "ID=%d\nNAME=%s\nPASSWORD=%s\n", &packetType,
					message.data.user_info.name, message.data.user_info.password);

				printf("\n注册请求： NAME=%s PASSWORD=%s\n\n",
					message.data.user_info.name, message.data.user_info.password);

				m_hServer->WriteMessage(message);
			}
			break;

			case Packet::Type::GET_ONLINE_USERS:
			{
				message.type = Server::Message::Type::GET_ONLINE_USERS;
				m_hServer->WriteMessage(message);
			}
			break;

			case Packet::Type::BATTLE_REQUEST:
			{
				int battleType = 0;
				sscanf(szRecvBuf, "ID=%d\nTY=%d\n", &packetType, &battleType);
				m_battle_stage.Clear();

				printf("收到一个比赛申请。\n");

				if (battleType == 0)
				{
					int pokemenId;
					Property opponent;
					sscanf(szRecvBuf, "ID=%d\nTY=%d\nID=%d\nTY=%d,HP=%d,AT=%d,DE=%d,AG=%d,BR=%d,CR=%d,HI=%d,PA=%d,EX=%d,LE=%d\n",
						&packetType, &m_battleType,
						&pokemenId,
						&opponent.type,
						&opponent.hpoints, &opponent.attack,
						&opponent.defense, &opponent.agility,
						&opponent.brea, &opponent.critical,
						&opponent.hitratio, &opponent.parryratio,
						&opponent.exp, &opponent.level);

					std::list<Pokemen::PokemenManager>::const_iterator it_pokemen;
					if ((it_pokemen = std::find_if(m_pokemens.begin(), m_pokemens.end(), [pokemenId](const Pokemen::PokemenManager& pokemen) {
						return pokemen.GetID() == pokemenId;
					})) != m_pokemens.end())
					{
						// 创建电脑小精灵
						Pokemen::PokemenManager secondPlayer{ opponent.type };
						secondPlayer.SetHpoints(opponent.hpoints);
						secondPlayer.SetAttack(opponent.attack);
						secondPlayer.SetDefense(opponent.defense);
						secondPlayer.SetAgility(opponent.agility);
						secondPlayer.SetBreak(opponent.brea);

						secondPlayer.SetCritical(opponent.critical);
						secondPlayer.SetHitratio(opponent.hitratio);
						secondPlayer.SetParryratio(opponent.parryratio);

						secondPlayer.SetExp(opponent.exp);
						secondPlayer.SetLevel(opponent.level);

						// 启动比赛场
						if (!m_battle_stage.IsRunning())
						{
							printf("与电脑的比赛开始。\n");

							m_battle_stage.AddPlayer(*it_pokemen, secondPlayer);
							m_battle_stage.Start();
							m_isReadBattleMessageClosed = false;

							delete m_readBattleMessageThread;
							m_readBattleMessageThread = new std::thread{ &UserManager::_read_battle_message_, this };
						}
					}
				}
				else if (battleType == 1)
				{

				}
			}
			break;

			case Packet::Type::DISCONNECT:
			{
				message.type = Server::Message::Type::USER_CLOSED;

				char szHostName[BUFSIZ];
				sscanf(szRecvBuf, "ID=%d\nHOST=%s\n", &packetType, szHostName);
				printf("\n登出请求： HOST=%s\n\n>", szHostName);

				m_hServer->WriteMessage(message);

				m_isClosed = true;
				SetEvent(m_recvClosedEvent);
				return;
			}
			break;

			default:
				break;
			}
		}
		else
		{	// close
			message.type = Server::Message::Type::USER_CLOSED;

			m_hServer->WriteMessage(message);

			m_isClosed = true;
			SetEvent(m_recvClosedEvent);
			return;
		}
	}

	SetEvent(m_recvClosedEvent);
}

void UserManager::_client_send_thread_()
{
	Packet sendPacket;
	int iSendBytes = 0;

	char szSendBuf[BUFLEN];

	while (!m_isClosed)
	{
		WaitForSingleObject(m_sendEvent, 2000);

		bool sendValid = false;
		m_sendMutex.lock();
		ResetEvent(m_sendEvent);

		if (!m_sendPacketQueue.empty())
		{
			sendValid = true;
			sendPacket = m_sendPacketQueue.front();
			m_sendPacketQueue.pop();
		}

		m_sendMutex.unlock();

		if (sendValid && sendPacket.type != Packet::Type::INVALID)
		{
			std::memset(szSendBuf, 0x0, sizeof(szSendBuf));
			sprintf(szSendBuf, "ID=%u\n", sendPacket.type);

			switch (sendPacket.type)
			{
			case Packet::Type::LAUNCH_SUCCESS:
			case Packet::Type::LAUNCH_FAILED:
			case Packet::Type::REGISTER_SUCCESS:
			case Packet::Type::REGISTER_FAILED:
			case Packet::Type::DISTRIBUTE_POKEMENS:
				break;

			case Packet::Type::INSERT_A_POKEMEN:
			case Packet::Type::UPDATE_USERS:
			case Packet::Type::SET_ONLINE_USERS:
			case Packet::Type::BATTLE_MESSAGE:
			{
				int iLen = (int)std::strlen(szSendBuf);
				std::memcpy(szSendBuf + iLen, sendPacket.data, std::strlen(sendPacket.data));
			}
			break;

			default:
				break;
			}

			iSendBytes = send(m_connectSocket,
				szSendBuf,
				(int)std::strlen(szSendBuf) + 1,
				0);

			if (iSendBytes == SOCKET_ERROR)
			{
				Server::Message message;
				message.type = Server::Message::Type::USER_CLOSED;
				message.id = m_clientAddr.sin_addr.S_un.S_addr;

				m_hServer->WriteMessage(message);

				m_isClosed = true;
				SetEvent(m_sendClosedEvent);
				return;
			}
		}
	}

	SetEvent(m_sendClosedEvent);
}