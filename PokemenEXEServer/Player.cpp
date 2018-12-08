#include "stdafx.h"

namespace Pokemen
{
	Property::Property()
	{
	}
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
			(int)id)
	{
	}

	BasePlayer::BasePlayer(const Property& prop) :
		m_property(prop)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::~BasePlayer()
	{
	}

	Id BasePlayer::GetId() const
	{
		return this->m_property.m_id;
	}

	PokemenType BasePlayer::GetType() const
	{
		return this->m_property.m_type;
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

	void BasePlayer::SetProperty(const Property& prop)
	{
		this->m_property = prop;
	}

/// <summary>
/// 
/// </summary>
	Value BasePlayer::IntervalValueCalculator(Value base, Value primary_affect, Value secondary_affect)
	{
		return base - static_cast<Value>(100.0 * std::ceil(std::sqrt((double)primary_affect) - std::sqrt((double)secondary_affect)));
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

	Value BasePlayer::ConvertValueByPercent(Value base, Value index)
	{
		return static_cast<Value>((double)base * ((double)index / 100.0));
	}
}