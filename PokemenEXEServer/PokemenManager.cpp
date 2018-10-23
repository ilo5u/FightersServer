#include "stdafx.h"

namespace Pokemen
{
	static char g_szMessage[BUFLEN];
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

		m_instance->SetLevel(other.GetLevel());
		m_instance->SetExp(other.GetExp());
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

		m_instance->SetLevel(other.GetLevel());
		m_instance->SetExp(other.GetExp());
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
	std::string PokemenManager::GetName() const
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

	bool PokemenManager::SetName(const std::string & name)
	{
		return m_instance->SetName(name);
	}

	bool PokemenManager::SetHpoints(int hpoints)
	{
		return m_instance->SetHpoints((int16_t)hpoints);
	}

	bool PokemenManager::SetAttack(int attack)
	{
		return m_instance->SetAttack((int16_t)attack);
	}

	bool PokemenManager::SetDefense(int defense)
	{
		return m_instance->SetDefense((int16_t)defense);
	}

	bool PokemenManager::SetAgility(int agility)
	{
		return m_instance->SetAgility((int16_t)agility);
	}

	bool PokemenManager::SetBreak(int brea)
	{
		return m_instance->SetBreak((int16_t)brea);
	}

	bool PokemenManager::SetCritical(int critical)
	{
		return m_instance->SetCritical((int16_t)critical);
	}

	bool PokemenManager::SetHitratio(int hitratio)
	{
		return m_instance->SetHitratio((int16_t)hitratio);
	}

	bool PokemenManager::SetParryratio(int parryratio)
	{
		return m_instance->SetParryratio((int16_t)parryratio);
	}

	bool PokemenManager::SetAnger(int anger)
	{
		return m_instance->SetAnger((int16_t)anger);
	}

	bool PokemenManager::SetLevel(int level)
	{
		return m_instance->SetLevel((int16_t)level);
	}

	bool PokemenManager::SetExp(int exp)
	{
		return m_instance->SetExp((int16_t)exp);
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

	BasePlayer::State operator&(const BasePlayer::State& l, const BasePlayer::State& r)
	{
		return static_cast<BasePlayer::State>(static_cast<uint16_t>(l) & static_cast<uint16_t>(r));
	}

	BasePlayer::State operator|(const BasePlayer::State & l, const BasePlayer::State & r)
	{
		return static_cast<BasePlayer::State>(static_cast<uint16_t>(l) | static_cast<uint16_t>(r));
	}

	bool operator!(const BasePlayer::State& s)
	{
		return static_cast<uint16_t>(s) != 0;
	}

	BattleStage::BattleStage() :
		m_first_player(PokemenType::TANK),
		m_second_player(PokemenType::TANK)
	{
		m_message_event = CreateEvent(NULL, FALSE, NULL, NULL);
		m_on_off_event = CreateEvent(NULL, TRUE, NULL, NULL);
	}

	BattleStage::~BattleStage()
	{
		CloseHandle(m_message_event);
	}

	void BattleStage::AddPlayer(const Pokemen::PokemenManager& firstPlayer, const Pokemen::PokemenManager& secondPlayer)
	{
		m_first_player = firstPlayer;
		m_second_player = secondPlayer;
	}

	void BattleStage::Start()
	{
		SetEvent(m_on_off_event);
		ResetEvent(m_message_event);
		m_is_battle_on_running = true;
		m_battle_thread = std::thread{ std::bind(&BattleStage::__run_battle__, this) };
	}

	void BattleStage::Pause()
	{
		ResetEvent(m_on_off_event);
	}

	void BattleStage::GoOn()
	{
		SetEvent(m_on_off_event);
	}

	void BattleStage::Clear()
	{
		m_is_battle_on_running = false;
		if (m_battle_thread.joinable())
			m_battle_thread.join();
		m_round_cnt = 0;
	}

	bool BattleStage::IsRunning() const
	{
		return m_is_battle_on_running;
	}

	int BattleStage::GetRoundCnt() const
	{
		return m_round_cnt;
	}

	int BattleStage::GetFirstPlayerId() const
	{
		return m_first_player.GetID();
	}

	int BattleStage::GetSecondPlayerId() const
	{
		return m_second_player.GetID();
	}

	BattleMessage BattleStage::ReadMessage()
	{
		WaitForSingleObject(m_message_event, 1000);
		BattleMessage message;
		m_message_mutex.lock();

		ResetEvent(m_message_event);
		if (!m_message_queue.empty())
		{
			message = m_message_queue.front();
			m_message_queue.pop();
		}

		m_message_mutex.unlock();
		return message;
	}

	void BattleStage::__run_battle__()
	{
		int break_of_first  = m_first_player.GetBreak();
		int break_of_second = m_second_player.GetBreak();

		m_round_cnt = 0;
		std::string message;
		while (!m_first_player.InState(BasePlayer::State::DEAD)
			&& !m_second_player.InState(BasePlayer::State::DEAD)
			&& m_is_battle_on_running)
		{
			++m_round_cnt;

			int min_span = min(break_of_first, break_of_second);
			Sleep(min_span);

			break_of_first -= min_span;
			break_of_second -= min_span;

			if (break_of_first == 0)
			{
				message = "FIRST=" + m_first_player.Attack(m_second_player);
				break_of_first = m_first_player.GetBreak();

				m_message_mutex.lock();

				m_message_queue.push(message);
				SetEvent(m_message_event);

				m_message_mutex.unlock();
			}
			if (break_of_second == 0)
			{
				message = "SECOND=" + m_second_player.Attack(m_first_player);
				break_of_second = m_second_player.GetBreak();

				m_message_mutex.lock();

				m_message_queue.push(message);
				SetEvent(m_message_event);

				m_message_mutex.unlock();
			}	// 将小精灵的所有属性值打包发送
			sprintf(g_szMessage, "RENEW:FIRST=%d,%d,%d,%d,%d,%d,%d,%d,%d\nSECOND=%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
				m_first_player.GetHpoints(), m_first_player.GetAttack(), m_first_player.GetDefense(), m_first_player.GetAgility(),
				m_first_player.GetBreak(), m_first_player.GetCritical(), m_first_player.GetHitratio(), m_first_player.GetParryratio(), m_first_player.GetAnger(),
				m_second_player.GetHpoints(), m_second_player.GetAttack(), m_second_player.GetDefense(), m_second_player.GetAgility(),
				m_second_player.GetBreak(), m_second_player.GetCritical(), m_second_player.GetHitratio(), m_second_player.GetParryratio(), m_second_player.GetAnger());

			m_message_mutex.lock();

			m_message_queue.push({ g_szMessage });
			SetEvent(m_message_event);

			m_message_mutex.unlock();

			// WaitForSingleObject(m_on_off_event, INFINITE);
		}

		message = "GAME END WITH ";
		if (m_first_player.InState(BasePlayer::State::DEAD))
			message += "0";
		else
			message += "1";

		m_message_mutex.lock();

		m_message_queue.push(message);
		SetEvent(m_message_event);

		m_message_mutex.unlock();

		m_is_battle_on_running = false;
	}

	BattleMessage::BattleMessage(const std::string& message) :
		wsOptions(message)
	{
	}
}