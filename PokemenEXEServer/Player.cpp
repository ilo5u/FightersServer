#include "stdafx.h"

namespace Pokemen
{
	/// <summary>
	/// 
	/// </summary>
	Property::Property(int type, const char name[],
		int hpoints, int attack, int defense, int agility,
		int interval, int critical, int hitratio, int parryratio,
		int exp, int level,
		int id) :
		m_id(id), m_type(static_cast<PokemenType>(type)), m_name(name),
		m_hpoints(hpoints), m_attack(attack), m_defense(defense), m_agility(agility),
		m_interval(interval), m_critical(critical), m_hitratio(hitratio), m_parryratio(parryratio),
		m_exp(exp), m_level(level)
	{
	}

	bool BasePlayer::InState(BasePlayer::State inState) const
	{
		return (static_cast<uint16_t>(this->m_state) & static_cast<uint16_t>(inState)) == 0 ? false : true;
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::BasePlayer(PokemenType type, String name,
		Value hitpoints, Value attack, Value defense, Value agility, 
		Value interval, Value critical, Value hitratio, Value parryratio,
		Id id) :
		m_property((int)type, name.c_str(),
		    (int)hitpoints, (int)attack, (int)defense, (int)agility,
			(int)interval, (int)critical, (int)hitratio, (int)parryratio,
			(int)0x0, (int)0x1,
			(int)id),
		m_anger(0x0), m_state(State::NORMAL),
		m_hpointsLimitation(hitpoints),
		m_effects(), m_stateRoundsCnt()
	{
	}

	BasePlayer::BasePlayer(const Property& prop) :
		m_property(prop),
		m_anger(0x0), m_state(State::NORMAL),
		m_hpointsLimitation(prop.m_hpoints),
		m_effects(), m_stateRoundsCnt()
	{
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::~BasePlayer()
	{
	}

	bool BasePlayer::SetAnger(int anger)
	{
		this->m_anger = anger;
		return false;
	}

	bool BasePlayer::Upgrade(int extra)
	{
		this->m_property.m_exp
			= std::min<Value>(
				this->m_property.m_exp + extra, 
				CommonBasicValues::levelLimitation * CommonBasicValues::exp - 1
				);

		Value oldlevel = this->m_property.m_level;
		this->m_property.m_level 
			= this->m_property.m_exp / CommonBasicValues::exp + 1;
		if (this->m_property.m_level > oldlevel)
		{
			_LevelUpPropertiesDistributor_();
			return true;
		}
		return false;
	}

	Id BasePlayer::GetId() const
	{
		return this->m_property.m_id;
	}

	PokemenType BasePlayer::GetType() const
	{
		return PokemenType();
	}

	String BasePlayer::GetName() const
	{
		return this->m_property.m_name;
	}

	Value BasePlayer::GetHpoints() const
	{
		return this->m_property.m_hpoints;
	}

	Value BasePlayer::GetAttack() const
	{
		return this->m_property.m_attack;
	}

	Value BasePlayer::GetDefense() const
	{
		return this->m_property.m_defense;
	}

	Value BasePlayer::GetAgility() const
	{
		return this->m_property.m_agility;
	}

	Value BasePlayer::GetInterval() const
	{
		return this->m_property.m_interval;
	}

	Value BasePlayer::GetCritical() const
	{
		return this->m_property.m_critical;
	}

	Value BasePlayer::GetHitratio() const
	{
		return this->m_property.m_hitratio;
	}

	Value BasePlayer::GetParryratio() const
	{
		return this->m_property.m_parryratio;
	}

	Value BasePlayer::GetExp() const
	{
		return this->m_property.m_exp;
	}

	Value BasePlayer::GetLevel() const
	{
		return this->m_property.m_level;
	}

	Value BasePlayer::GetAnger() const
	{
		return this->m_anger;
	}

	bool BasePlayer::SetDizzyingRounds(Value rounds)
	{
		this->m_stateRoundsCnt.dizzying = rounds;
		return false;
	}

	bool BasePlayer::SetBleedRounds(Value rounds)
	{
		this->m_stateRoundsCnt.bleed = rounds;
		return false;
	}

	bool BasePlayer::SetSunderedRounds(Value rounds)
	{
		this->m_stateRoundsCnt.sundered = rounds;
		return false;
	}

	bool BasePlayer::SetSlowedRounds(Value rounds)
	{
		this->m_stateRoundsCnt.slowed = rounds;
		return false;
	}

	bool BasePlayer::SetSilentRounds(Value rounds)
	{
		this->m_stateRoundsCnt.silent = rounds;
		return false;
	}

	bool BasePlayer::SetWeakenRounds(Value rounds)
	{
		this->m_stateRoundsCnt.weaken = rounds;
		return false;
	}

	/// <summary>
/// 
/// </summary>
	Value BasePlayer::IntervalValueCalculator(Value base, Value primary_affect, Value secondary_affect)
	{
		return base - static_cast<Value>(100.0 * std::sqrt((double)primary_affect) - 100.0 * std::sqrt((double)secondary_affect));
	}

	/// <summary>
	/// 
	/// </summary>
	Value BasePlayer::CriticalValueCalculator(Value base, Value affect)
	{
		return base + static_cast<Value>(4.0 * std::sqrt((double)affect));
	}

	/// <summary>
	/// 
	/// </summary>
	Value BasePlayer::HitratioValueCalculator(Value base, Value primary_affect, Value secondary_affect)
	{
		return base - static_cast<Value>(4.0 * std::sqrt((double)primary_affect) + std::sqrt(std::sqrt((double)secondary_affect)));
	}

	/// <summary>
	/// 
	/// </summary>
	Value BasePlayer::ParryratioValueCalculator(Value base, Value primary_affect, Value secondary_affect)
	{
		return base + static_cast<Value>(4.0 * std::sqrt((double)primary_affect) - std::sqrt((double)secondary_affect));
	}

	/// <summary>
	/// 
	/// </summary>
	Value BasePlayer::AttackDamageCalculator(Value primary_affect, Value secondary_affect)
	{
		return std::max<Value>(0x1, static_cast<Value>(((double)primary_affect * (double)primary_affect) / ((double)primary_affect + (double)secondary_affect)));
	}

	/// <summary>
	/// 
	/// </summary>
	Value BasePlayer::HealingHpointsCalculator(Value primary_affect, Value secondary_affect)
	{
		return static_cast<Value>((double)primary_affect * ((double)secondary_affect / 200.0));
	}

	/// <summary>
	/// 
	/// </summary>
	Value BasePlayer::BloodingDamageCalculator(Value primary_affect, Value secondary_affect)
	{
		return static_cast<Value>(((double)primary_affect * 10.0) / std::sqrt((double)secondary_affect));
	}

	Value BasePlayer::ConvertValueByPercent(Value base, Value index)
	{
		return static_cast<Value>((double)base * ((double)index / 100.0));
	}

	bool BasePlayer::AddState(BasePlayer::State newState)
	{
		this->m_state 
			= static_cast<BasePlayer::State>(static_cast<uint16_t>(this->m_state) | static_cast<uint16_t>(newState));
		return false;
	}

	bool BasePlayer::SubState(BasePlayer::State oldState)
	{
		this->m_state 
			= static_cast<BasePlayer::State>(static_cast<uint16_t>(this->m_state) & (~static_cast<uint16_t>(oldState)));
		return false;
	}

}