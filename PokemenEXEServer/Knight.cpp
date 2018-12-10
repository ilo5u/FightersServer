#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfKnight{ "Charizard", "Squirtle", "Wartortle", "Blastoise" };

	Knight::Career::Career(Career::Type type) :
		type(type)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Knight::Knight(int level) :
		BasePlayer(
			PokemenType::KNIGHT, namesOfKnight[_Random(namesOfKnight.size())],
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
				this->m_property.m_defense
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
				this->m_property.m_defense
			);
		this->m_property.m_parryratio
			= ParryratioValueCalculator(
				CommonBasicValues::parryratio,
				this->m_property.m_agility,
				this->m_property.m_attack
			);
	}

	Knight::Knight(const Property& prop, Career::Type career) :
		BasePlayer(prop),
		m_career(career)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Knight::~Knight()
	{
	}

	Knight::Career::Type Knight::GetCareer() const
	{
		return this->m_career.type;
	}

	bool Knight::Promote(Career::Type career)
	{
		if (this->m_career.type == Career::Type::Normal)
		{
			this->m_career.type = career;
			int bonus = (this->m_property.m_level - 8) * 10 + 100;
			switch (this->m_career.type)
			{
			case Career::Type::Ares:
			{
				this->m_property.m_attack
					+= ConvertValueByPercent(ConvertValueByPercent(this->m_property.m_attack, Career::Ares::damageIncIndex), bonus);
				this->m_property.m_interval
					+= Career::Ares::intervalIncIndex;
			}
			break;

			case Career::Type::Athena:
			{
				this->m_property.m_defense
					+= ConvertValueByPercent(ConvertValueByPercent(this->m_property.m_defense, Career::Athena::defenseIncIndex), bonus);
				this->m_property.m_attack
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Athena::damageDecIndex);
				this->m_property.m_interval
					+= Career::Athena::intervalDecIndex;
			}
			break;

			default:
				break;
			}
		}
		return false;
	}
}