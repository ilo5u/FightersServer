#include "stdafx.h"

namespace Pokemen
{
	/// <summary>
	/// 
	/// </summary>
	PokemenProperty::PokemenProperty(String name, Value hitpoints, Value attack, Value defense, Value agility, PokemenType type) :
		m_name(name),
		m_hpoints(hitpoints), m_attack(attack), m_defense(defense),	m_agility(agility),
		m_type(type),
		m_exp(0), m_level(1)
	{
		m_break      = BreakValueCalculator(breakBase, m_agility, m_attack);
		m_critical   = CriticalValueCalculator(criticalBase, m_agility);
		m_hitratio   = HitratioValueCalculator(hitratioBase, m_agility, m_hpoints);
		m_parryratio = ParryratioValueCalculator(parryratioBase, m_agility, m_defense);
	}

	/// <summary>
	/// 
	/// </summary>
	PokemenProperty::~PokemenProperty()
	{
	}

	int PokemenProperty::GetID() const
	{
		return m_id;
	}

	String PokemenProperty::GetName() const
	{
		return m_name;
	}

	Value PokemenProperty::GetHpoints() const
	{
		return m_hpoints;
	}

	Value PokemenProperty::GetAttack() const
	{
		return m_attack;
	}

	Value PokemenProperty::GetDefense() const
	{
		return m_defense;
	}

	Value PokemenProperty::GetAgility() const
	{
		return m_agility;
	}

	Value PokemenProperty::GetBreak() const
	{
		return m_break;
	}

	Value PokemenProperty::GetCritical() const
	{
		return m_critical;
	}

	Value PokemenProperty::GetHitratio() const
	{
		return m_hitratio;
	}

	Value PokemenProperty::GetParryratio() const
	{
		return m_parryratio;
	}

	PokemenType PokemenProperty::GetType() const
	{
		return m_type;
	}

	Value PokemenProperty::GetExp() const
	{
		return m_exp;
	}

	Value PokemenProperty::GetLevel() const
	{
		return m_level;
	}

	bool PokemenProperty::SetID(int id)
	{
		m_id = id;
		return false;
	}

	bool PokemenProperty::SetName(const String& name)
	{
		m_name = name;
		return false;
	}

	bool PokemenProperty::SetHpoints(Value hpoints)
	{
		m_hpoints = hpoints;
		return false;
	}

	bool PokemenProperty::SetAttack(Value attack)
	{
		m_attack = attack;
		return false;
	}

	bool PokemenProperty::SetDefense(Value defense)
	{
		m_defense = defense;
		return false;
	}

	bool PokemenProperty::SetAgility(Value agility)
	{
		m_agility = agility;
		return false;
	}

	bool PokemenProperty::SetBreak(Value brea)
	{
		m_break = brea;
		return false;
	}

	bool PokemenProperty::SetCritical(Value critical)
	{
		m_critical = critical;
		return false;
	}

	bool PokemenProperty::SetHitratio(Value hitratio)
	{
		m_hitratio = hitratio;
		return false;
	}

	bool PokemenProperty::SetParryratio(Value parryratio)
	{
		m_parryratio = parryratio;
		return false;
	}

	bool PokemenProperty::SetExp(Value exp)
	{
		m_exp = exp;
		return false;
	}

	bool PokemenProperty::SetLevel(Value level)
	{
		m_level = level;
		return false;
	}

	/// <summary>
/// 
/// </summary>
	Value PokemenProperty::BreakValueCalculator(Value base, Value primary_affect, Value secondary_affect)
	{
		return base - static_cast<Value>(100.0 * std::sqrt((double)primary_affect) + 100.0 * std::sqrt((double)secondary_affect));
	}

	/// <summary>
	/// 
	/// </summary>
	Value PokemenProperty::CriticalValueCalculator(Value base, Value affect)
	{
		return base + static_cast<Value>(4.0 * std::sqrt((double)affect));
	}

	/// <summary>
	/// 
	/// </summary>
	Value PokemenProperty::HitratioValueCalculator(Value base, Value primary_affect, Value secondary_affect)
	{
		return base - static_cast<Value>(4.0 * std::sqrt((double)primary_affect) + std::sqrt(std::sqrt((double)secondary_affect)));
	}

	/// <summary>
	/// 
	/// </summary>
	Value PokemenProperty::ParryratioValueCalculator(Value base, Value primary_affect, Value secondary_affect)
	{
		return base + static_cast<Value>(4.0 * std::sqrt((double)primary_affect) - std::sqrt((double)secondary_affect));
	}

	/// <summary>
	/// 
	/// </summary>
	Value PokemenProperty::AttackDamageCalculator(Value primary_affect, Value secondary_affect)
	{
		return static_cast<Value>(((double)primary_affect * (double)primary_affect) / ((double)primary_affect + (double)secondary_affect));
	}

	/// <summary>
	/// 
	/// </summary>
	Value PokemenProperty::HealingHpointsCalculator(Value primary_affect, Value secondary_affect)
	{
		return static_cast<Value>((double)primary_affect * ((double)secondary_affect / 200.0));
	}

	/// <summary>
	/// 
	/// </summary>
	Value PokemenProperty::BloodingDamageCalculator(Value primary_affect, Value secondary_affect)
	{
		return static_cast<Value>(((double)primary_affect * 10.0) / std::sqrt((double)secondary_affect));
	}

	bool BasePlayer::InState(BasePlayer::State inState) const
	{
		return (static_cast<uint16_t>(this->m_state) & static_cast<uint16_t>(inState)) == 0 ? false : true;
	}

	bool BasePlayer::AddState(BasePlayer::State newState)
	{
		this->m_state = static_cast<BasePlayer::State>(static_cast<uint16_t>(this->m_state) | static_cast<uint16_t>(newState));
		return false;
	}

	bool BasePlayer::SubState(BasePlayer::State oldState)
	{
		this->m_state = static_cast<BasePlayer::State>(static_cast<uint16_t>(this->m_state) & (~static_cast<uint16_t>(oldState)));
		return false;
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::BasePlayer(String name, Value hitpoints, Value attack, Value defense, Value agility, PokemenType type) :
		PokemenProperty(name, hitpoints, attack, defense, agility, type),
		m_state(State::NORMAL),
		m_anger(0),
		m_hpointsLimitation(hitpoints), 
		m_attackOriginal(attack), m_defenseOriginal(defense), m_agilityOriginal(agility)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::~BasePlayer()
	{
	}

	/// <summary>
	/// 
	/// </summary>
	String BasePlayer::Attack(HCPokemenPlayer oppoent)
	{
		throw std::exception("Not implement class in method Attack()");
		return false;
	}

	/// <summary>
	/// 
	/// </summary>
	Value BasePlayer::IsAttacked(Value damage)
	{
		throw std::exception("Method IsAttacked() in CPokemenPlayer is not implement.");
	}

	/// <summary>
	/// 
	/// </summary>
	bool BasePlayer::SetPrimarySkill(const SkillBase& skill)
	{
		throw std::exception("Method IsAttacked() in CPokemenPlayer is not implement.");
	}

	bool BasePlayer::SetAnger(int anger)
	{
		this->m_anger = anger;
		return false;
	}

	bool BasePlayer::Update(int extra)
	{
		this->m_exp = min(this->m_exp + extra, levelLimitation * expBase - 1);

		Value oldlevel = this->m_level;
		this->m_level = this->m_exp / expBase + 1;
		if (this->m_level > oldlevel)
		{
			_LevelUpPropertiesDistributor_();
			return true;
		}
		return false;
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::State BasePlayer::GetState() const
	{
		return this->m_state;
	}

	Value BasePlayer::GetAnger() const
	{
		return this->m_anger;
	}

	void BasePlayer::_LevelUpPropertiesDistributor_()
	{
	}
}