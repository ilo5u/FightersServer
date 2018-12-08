#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfGuardian{ "Caterpie", "Metapod", "Butterfree", "Weedle" };

	Guardian::Career::Career(Career::Type type) :
		type(type)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Guardian::Guardian(int level) :
		BasePlayer(
			PokemenType::GUARDIAN, namesOfGuardian[_Random(namesOfGuardian.size())],
			BasicProperties::hitpoints + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::attack + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::defense + _Random(CommonBasicValues::propertyOffset),
			BasicProperties::agility + _Random(CommonBasicValues::propertyOffset),
			0x0, 0x0, 0x0, 0x0
		),
		m_career(Career::Type::Normal)
	{
		this->m_property.m_interval
			= IntervalValueCalculator(
				CommonBasicValues::interval,
				this->m_property.m_agility,
				this->m_property.m_attack
			);
		this->m_property.m_critical
			= CriticalValueCalculator(
				CommonBasicValues::critical,
				this->m_property.m_agility
			);
		this->m_property.m_hitratio
			= HitratioValueCalculator(
				CommonBasicValues::hitratio,
				this->m_property.m_agility,
				this->m_property.m_attack
			);
		this->m_property.m_parryratio
			= ParryratioValueCalculator(
				CommonBasicValues::parryratio,
				this->m_property.m_agility,
				this->m_property.m_defense
			);
	}

	Guardian::Guardian(const Property& prop, Career::Type career) :
		BasePlayer(prop),
		m_career(career)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Guardian::~Guardian()
	{
	}

	Guardian::Career::Type Guardian::GetCareer() const
	{
		return this->m_career.type;
	}

	bool Guardian::Promote(Career::Type career)
	{
		if (this->m_career.type == Career::Type::Normal)
		{
			this->m_career.type = career;
			switch (this->m_career.type)
			{
			case Career::Type::Paladin:
			{
				this->m_property.m_defense 
					+= ConvertValueByPercent(this->m_property.m_defense, Career::Paladin::defenseIncIndex);
				this->m_property.m_attack
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Paladin::damageDecIndex);
				this->m_property.m_agility 
					+= (this->m_property.m_agility, Career::Paladin::agilityDecIndex);
			}
			break;

			case Career::Type::Joker:
			{
				this->m_property.m_defense 
					+= (this->m_property.m_defense, Career::Joker::defenseDecIndex);
			}
			break;

			default:
				break;
			}
		}
		return false;
	}
}
