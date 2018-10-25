#include "stdafx.h"

namespace Pokemen
{
	static char battleMessage[BUFLEN];
	/// <summary>
	/// 
	/// </summary>
	PokemenManager::PokemenManager(PokemenType type, int level) :
		m_instance(nullptr)
	{
		if (type == PokemenType::DEFAULT)
			type = static_cast<PokemenType>(_Random(1, 4));

		switch (type)
		{

		case PokemenType::TANK:
			m_instance = new Tank{ Tank::Skill::Type::DOUBLE_ANGRY, level };
			break;

		case PokemenType::LION:
			m_instance = new Lion{ Lion::Skill::Type::SUNDER_ARM, level };
			break;

		case PokemenType::HEDGEHOG:
			m_instance = new Hedgehog{ Hedgehog::Skill::Type::SUNK_IN_SILENCE, level };
			break;

		case PokemenType::EAGLE:
			m_instance = new Eagle{ Eagle::Skill::Type::TEARING, level };
			break;

		default:
			throw std::exception("Create a player pokemen with a error type.");
			break;
		}
	}

	PokemenManager::PokemenManager(const PokemenManager& other)
	{
		switch (other.m_instance->GetType())
		{
		case PokemenType::TANK:
			m_instance = new Tank{ Tank::Skill::Type::DOUBLE_ANGRY };
			break;

		case PokemenType::LION:
			m_instance = new Lion{ Lion::Skill::Type::SUNDER_ARM };
			break;

		case PokemenType::HEDGEHOG:
			m_instance = new Hedgehog{ Hedgehog::Skill::Type::SUNK_IN_SILENCE };
			break;

		case PokemenType::EAGLE:
			m_instance = new Eagle{ Eagle::Skill::Type::TEARING };
			break;

		default:
			throw std::exception("Create a player pokemen with a error type.");
			break;
		}

		m_instance->SetID(other.GetID());
		m_instance->SetName(other.GetName());

		m_instance->SetHpoints(other.GetHpoints());
		m_instance->SetAttack(other.GetAttack());
		m_instance->SetDefense(other.GetDefense());
		m_instance->SetAgility(other.GetAgility());

		m_instance->SetBreak(other.GetBreak());
		m_instance->SetCritical(other.GetCritical());
		m_instance->SetHitratio(other.GetHitratio());
		m_instance->SetParryratio(other.GetParryratio());
		m_instance->SetAnger(other.GetAnger());

		m_instance->SetExp(other.GetExp());
		m_instance->SetLevel(other.GetLevel());
	}

	PokemenManager::PokemenManager(PokemenManager&& other)
	{
		m_instance = other.m_instance;
		other.m_instance = nullptr;
	}

	PokemenManager& PokemenManager::operator=(const PokemenManager& other)
	{
		switch (other.m_instance->GetType())
		{
		case PokemenType::TANK:
			m_instance = new Tank{ Tank::Skill::Type::DOUBLE_ANGRY };
			break;

		case PokemenType::LION:
			m_instance = new Lion{ Lion::Skill::Type::SUNDER_ARM };
			break;

		case PokemenType::HEDGEHOG:
			m_instance = new Hedgehog{ Hedgehog::Skill::Type::SUNK_IN_SILENCE };
			break;

		case PokemenType::EAGLE:
			m_instance = new Eagle{ Eagle::Skill::Type::TEARING };
			break;

		default:
			throw std::exception("Create a player pokemen with a error type.");
			break;
		}

		m_instance->SetID(other.GetID());
		m_instance->SetName(other.GetName());

		m_instance->SetHpoints(other.GetHpoints());
		m_instance->SetAttack(other.GetAttack());
		m_instance->SetDefense(other.GetDefense());
		m_instance->SetAgility(other.GetAgility());

		m_instance->SetBreak(other.GetBreak());
		m_instance->SetCritical(other.GetCritical());
		m_instance->SetHitratio(other.GetHitratio());
		m_instance->SetParryratio(other.GetParryratio());
		m_instance->SetAnger(other.GetAnger());

		m_instance->SetExp(other.GetExp());
		m_instance->SetLevel(other.GetLevel());
		return *this;
	}

	PokemenManager& PokemenManager::operator=(PokemenManager&& other)
	{
		m_instance = other.m_instance;
		other.m_instance = nullptr;
		return *this;
	}

	/// <summary>
	/// 
	/// </summary>
	PokemenManager::~PokemenManager()
	{
		delete m_instance;
		m_instance = nullptr;
	}

	int PokemenManager::GetID() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetID();
	}

	/// <summary>
	/// 
	/// </summary>
	String PokemenManager::GetName() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetName();
	}

	/// <summary>
	/// 
	/// </summary>
	int PokemenManager::GetHpoints() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetHpoints();
	}

	/// <summary>
	/// 
	/// </summary>
	int PokemenManager::GetAttack() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetAttack();
	}

	/// <summary>
	/// 
	/// </summary>
	int PokemenManager::GetDefense() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetDefense();
	}

	/// <summary>
	/// 
	/// </summary>
	int PokemenManager::GetAgility() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetAgility();
	}

	/// <summary>
	/// 
	/// </summary>
	int PokemenManager::GetBreak() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetBreak();
	}

	/// <summary>
	/// 
	/// </summary>
	int PokemenManager::GetCritical() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetCritical();
	}

	/// <summary>
	/// 
	/// </summary>
	int PokemenManager::GetHitratio() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetHitratio();
	}

	/// <summary>
	/// 
	/// </summary>
	int PokemenManager::GetParryratio() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetParryratio();
	}

	int PokemenManager::GetAnger() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetAnger();
	}

	int PokemenManager::GetLevel() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetLevel();
	}

	int PokemenManager::GetExp() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetExp();
	}

	int PokemenManager::SetID(int id)
	{
		return m_instance->SetID(id);
	}

	bool PokemenManager::SetName(const String& name)
	{
		return m_instance->SetName(name);
	}

	bool PokemenManager::SetHpoints(int hpoints)
	{
		return m_instance->SetHpoints((Value)hpoints);
	}

	bool PokemenManager::SetAttack(int attack)
	{
		return m_instance->SetAttack((Value)attack);
	}

	bool PokemenManager::SetDefense(int defense)
	{
		return m_instance->SetDefense((Value)defense);
	}

	bool PokemenManager::SetAgility(int agility)
	{
		return m_instance->SetAgility((Value)agility);
	}

	bool PokemenManager::SetBreak(int brea)
	{
		return m_instance->SetBreak((Value)brea);
	}

	bool PokemenManager::SetCritical(int critical)
	{
		return m_instance->SetCritical((Value)critical);
	}

	bool PokemenManager::SetHitratio(int hitratio)
	{
		return m_instance->SetHitratio((Value)hitratio);
	}

	bool PokemenManager::SetParryratio(int parryratio)
	{
		return m_instance->SetParryratio((Value)parryratio);
	}

	bool PokemenManager::SetAnger(int anger)
	{
		return m_instance->SetAnger((Value)anger);
	}

	bool PokemenManager::SetLevel(int level)
	{
		return m_instance->SetLevel((Value)level);
	}

	bool PokemenManager::SetExp(int exp)
	{
		return m_instance->SetExp((Value)exp);
	}

	PokemenType PokemenManager::GetType() const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->GetType();
	}

	bool PokemenManager::InState(BasePlayer::State nowState) const
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return m_instance->InState(nowState);
	}

	/// <summary>
	/// 
	/// </summary>
	std::string PokemenManager::Attack(PokemenManager& opponent)
	{
		if (m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return static_cast<PBasePlayer>(this->m_instance)->Attack(static_cast<PBasePlayer>(opponent.m_instance));
	}

	BattleStage::BattleStage() :
		m_roundsCnt(0),
		m_firstPlayer(PokemenType::DEFAULT), m_secondPlayer(PokemenType::DEFAULT),
		m_messages(), m_messagesMutex(), m_messagesEvent(nullptr),
		m_stateEvent(nullptr), m_battleThread(), m_isBattleRunnig(false)
	{
		m_messagesEvent = CreateEvent(NULL, FALSE, NULL, NULL);
		m_stateEvent    = CreateEvent(NULL, FALSE, NULL, NULL);
	}

	BattleStage::~BattleStage()
	{
		CloseHandle(m_messagesEvent);
	}

	void BattleStage::SetPlayers(const Pokemen::PokemenManager& firstPlayer, const Pokemen::PokemenManager& secondPlayer)
	{
		m_firstPlayer = firstPlayer;
		m_secondPlayer = secondPlayer;
	}

	void BattleStage::Start()
	{
		SetEvent(m_stateEvent);
		ResetEvent(m_messagesEvent);
		m_isBattleRunnig = true;

		m_battleThread = std::move(std::thread{ std::bind(&BattleStage::_RunBattle_, this) });
	}

	bool BattleStage::Pause()
	{
		ResetEvent(m_stateEvent);
		return m_isBattleRunnig;
	}

	bool BattleStage::GoOn()
	{
		SetEvent(m_stateEvent);
		return m_isBattleRunnig;
	}

	void BattleStage::Clear()
	{
		m_isBattleRunnig = false;
		if (m_battleThread.joinable())
			m_battleThread.join();
		m_roundsCnt = 0;
	}

	bool BattleStage::IsRunning() const
	{
		return m_isBattleRunnig;
	}

	int BattleStage::GetRoundCnt() const
	{
		return m_roundsCnt;
	}

	int BattleStage::GetFirstPlayerId() const
	{
		return m_firstPlayer.GetID();
	}

	int BattleStage::GetSecondPlayerId() const
	{
		return m_secondPlayer.GetID();
	}

	BattleMessage BattleStage::ReadMessage()
	{
		WaitForSingleObject(m_messagesEvent, 2000);
		BattleMessage message;
		m_messagesMutex.lock();

		ResetEvent(m_messagesEvent);
		if (!m_messages.empty())
		{
			message = m_messages.front();
			m_messages.pop();
		}

		m_messagesMutex.unlock();
		return message;
	}

	void BattleStage::_RunBattle_()
	{
		int break_of_first  = m_firstPlayer.GetBreak();
		int break_of_second = m_secondPlayer.GetBreak();

		m_roundsCnt = 0;
		String message;
		while (!m_firstPlayer.InState(BasePlayer::State::DEAD)
			&& !m_secondPlayer.InState(BasePlayer::State::DEAD)
			&& m_isBattleRunnig)
		{
			++m_roundsCnt;

			int min_span = std::min<int>(break_of_first, break_of_second);
			Sleep(min_span);

			break_of_first  -= min_span;
			break_of_second -= min_span;

			if (break_of_first == 0)
			{
				message = "FIRST=" + m_firstPlayer.Attack(m_secondPlayer);
				break_of_first = m_firstPlayer.GetBreak();

				m_messagesMutex.lock();

				m_messages.push(message);
				SetEvent(m_messagesEvent);

				m_messagesMutex.unlock();
			}
			if (break_of_second == 0)
			{
				message = "SECOND=" + m_secondPlayer.Attack(m_firstPlayer);
				break_of_second = m_secondPlayer.GetBreak();

				m_messagesMutex.lock();

				m_messages.push(message);
				SetEvent(m_messagesEvent);

				m_messagesMutex.unlock();
			}	// 将小精灵的所有属性值打包发送
			sprintf(battleMessage, "RENEW:FIRST=%d,%d,%d,%d,%d,%d,%d,%d,%d\nSECOND=%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
				m_firstPlayer.GetHpoints(), m_firstPlayer.GetAttack(), m_firstPlayer.GetDefense(), m_firstPlayer.GetAgility(),
				m_firstPlayer.GetBreak(), m_firstPlayer.GetCritical(), m_firstPlayer.GetHitratio(), m_firstPlayer.GetParryratio(), m_firstPlayer.GetAnger(),
				m_secondPlayer.GetHpoints(), m_secondPlayer.GetAttack(), m_secondPlayer.GetDefense(), m_secondPlayer.GetAgility(),
				m_secondPlayer.GetBreak(), m_secondPlayer.GetCritical(), m_secondPlayer.GetHitratio(), m_secondPlayer.GetParryratio(), m_secondPlayer.GetAnger());

			m_messagesMutex.lock();

			m_messages.push({ battleMessage });
			SetEvent(m_messagesEvent);

			m_messagesMutex.unlock();

			WaitForSingleObject(m_stateEvent, INFINITE);
		}

		message = "GAME END WITH ";
		if (m_firstPlayer.InState(BasePlayer::State::DEAD))
			message += "0";
		else
			message += "1";

		m_messagesMutex.lock();

		m_messages.push(message);
		SetEvent(m_messagesEvent);

		m_messagesMutex.unlock();

		m_isBattleRunnig = false;
	}

	BattleMessage::BattleMessage(const String& message) :
		options(message)
	{
	}
}