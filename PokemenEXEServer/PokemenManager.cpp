#include "stdafx.h"
#include "PokemenManager.h"

namespace Pokemen
{
	static char g_wszMessage[1024];
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
			break;

		case PokemenType::LION:
			m_instance = new Lion{ Lion::Skill::Type::SUNDER_ARM };
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
			break;

		case PokemenType::HEDGEHOG:
			m_instance = new Hedgehog{ Hedgehog::Skill::Type::SUNK_IN_SILENCE };
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
			break;

		case PokemenType::EAGLE:
			m_instance = new Eagle{ Eagle::Skill::Type::TEARING };
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
			break;

		default:
			throw std::exception("Create a player pokemen with a error type.");
			break;
		}
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
			break;

		case PokemenType::LION:
			m_instance = new Lion{ Lion::Skill::Type::SUNDER_ARM };
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
			break;

		case PokemenType::HEDGEHOG:
			m_instance = new Hedgehog{ Hedgehog::Skill::Type::SUNK_IN_SILENCE };
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
			break;

		case PokemenType::EAGLE:
			m_instance = new Eagle{ Eagle::Skill::Type::TEARING };
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
			break;

		default:
			throw std::exception("Create a player pokemen with a error type.");
			break;
		}
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
		m_player_1(PokemenType::TANK),
		m_player_2(PokemenType::TANK)
	{
		m_message_event = CreateEvent(NULL, FALSE, NULL, NULL);
		m_on_off_event = CreateEvent(NULL, TRUE, NULL, NULL);
	}

	BattleStage::~BattleStage()
	{
		CloseHandle(m_message_event);
	}

	void BattleStage::AddPlayer(const Pokemen::PokemenManager& player_1, const Pokemen::PokemenManager& player_2)
	{
		m_player_1 = player_1;
		m_player_2 = player_2;
	}

	void BattleStage::Start()
	{
		SetEvent(m_on_off_event);
		ResetEvent(m_message_event);
		std::thread battle_thread{ std::bind(&BattleStage::__run_battle__, this) };
		battle_thread.detach();
	}

	void BattleStage::Pause()
	{
		ResetEvent(m_on_off_event);
	}

	void BattleStage::GoOn()
	{
		SetEvent(m_on_off_event);
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

	PokemenManager BattleStage::GetPlayer_1()
	{
		return m_player_1;
	}

	PokemenManager BattleStage::GetPlayer_2()
	{
		return m_player_2;
	}

	void BattleStage::__run_battle__()
	{
		int span_player_1 = m_player_1.GetBreak();
		int span_player_2 = m_player_2.GetBreak();

		std::string message;

		while (!m_player_1.InState(BasePlayer::State::DEAD)
			&& !m_player_2.InState(BasePlayer::State::DEAD))
		{
			std::memset(g_wszMessage, 0x0, sizeof(g_wszMessage));
			std::sprintf(g_wszMessage, 
				"Property %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
				m_player_1.GetHpoints(), m_player_2.GetHpoints(),
				m_player_1.GetAttack(), m_player_2.GetAttack(), 
				m_player_1.GetDefense(), m_player_2.GetDefense(), 
				m_player_1.GetAgility(), m_player_2.GetAgility(), 
				m_player_1.GetBreak(), m_player_2.GetBreak(), 
				m_player_1.GetCritical(), m_player_2.GetCritical(), 
				m_player_1.GetHitratio(), m_player_2.GetHitratio(), 
				m_player_1.GetParryratio(), m_player_2.GetParryratio(), 
				m_player_1.GetAnger(), m_player_2.GetAnger());
			message = g_wszMessage;

			m_message_mutex.lock();

			m_message_queue.push(message);
			SetEvent(m_message_event);

			m_message_mutex.unlock();

			int min_span = min(span_player_1, span_player_2);
			Sleep(min_span);

			span_player_1 -= min_span;
			span_player_2 -= min_span;

			if (span_player_1 == 0)
			{
				message = m_player_1.GetName();
				message += ": ";
				message += m_player_1.Attack(m_player_2);
				span_player_1 = m_player_1.GetBreak();

				m_message_mutex.lock();

				m_message_queue.push(message);
				SetEvent(m_message_event);

				m_message_mutex.unlock();
			}
			if (span_player_2 == 0)
			{
				message = m_player_2.GetName();
				message += ": ";
				message += m_player_2.Attack(m_player_1);
				span_player_2 = m_player_2.GetBreak();

				m_message_mutex.lock();

				m_message_queue.push(message);
				SetEvent(m_message_event);

				m_message_mutex.unlock();
			}

			WaitForSingleObject(m_on_off_event, INFINITE);
		}

		std::memset(g_wszMessage, 0x0, sizeof(g_wszMessage));
		std::sprintf(g_wszMessage,
			"Property %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			m_player_1.GetHpoints(), m_player_2.GetHpoints(),
			m_player_1.GetAttack(), m_player_2.GetAttack(),
			m_player_1.GetDefense(), m_player_2.GetDefense(),
			m_player_1.GetAgility(), m_player_2.GetAgility(),
			m_player_1.GetBreak(), m_player_2.GetBreak(),
			m_player_1.GetCritical(), m_player_2.GetCritical(),
			m_player_1.GetHitratio(), m_player_2.GetHitratio(),
			m_player_1.GetParryratio(), m_player_2.GetParryratio(),
			m_player_1.GetAnger(), m_player_2.GetAnger());
		message = g_wszMessage;

		m_message_mutex.lock();

		m_message_queue.push(message);
		SetEvent(m_message_event);

		m_message_mutex.unlock();

		m_message_mutex.lock();

		m_message_queue.push({ "GAME END" });
		SetEvent(m_message_event);

		m_message_mutex.unlock();
	}

	BattleMessage::BattleMessage(const std::string& message) :
		wsOptions(message)
	{
	}
}