#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfAssassin{ "Kakuna", "Beedrill", "Pidgey", "Pidgeotto" };

	Assassin::Career::Career(Career::Type type) :
		type(type)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Assassin::Assassin(int level) :
		BasePlayer(
			PokemenType::ASSASSIN, namesOfAssassin[_Random(namesOfAssassin.size())],
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
				this->m_property.m_attack
			);
		this->m_property.m_parryratio
			= ParryratioValueCalculator(
				CommonBasicValues::parryratio,
				this->m_property.m_agility,
				this->m_property.m_defense
			);
	}

	Assassin::Assassin(const Property& prop, Career::Type career) :
		BasePlayer(prop),
		m_career(career)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	Assassin::~Assassin()
	{
	}

	Assassin::Career::Type Assassin::GetCareer() const
	{
		return this->m_career.type;
	}

	bool Assassin::Promote(Career::Type career)
	{
		if (this->m_career.type == Career::Type::Normal)
		{
			this->m_career.type = career;
			switch (this->m_career.type)
			{
			case Career::Type::Yodian:
			{
				this->m_property.m_attack
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Yodian::damageDecIndex);
				this->m_property.m_interval 
					+= Career::Yodian::intervalDecIndex;
				this->m_property.m_hpoints 
					+= ConvertValueByPercent(this->m_property.m_hpoints, Career::Yodian::hpointsDecIndex);
				this->m_property.m_agility 
					+= ConvertValueByPercent(this->m_property.m_agility, Career::Yodian::agilityIncIndex);
			}
			break;

			case Career::Type::Michelle:
			{
				this->m_property.m_attack 
					+= ConvertValueByPercent(this->m_property.m_attack, Career::Michelle::damageIncIndex);
				this->m_property.m_defense
					+= ConvertValueByPercent(this->m_property.m_defense, Career::Michelle::defenseIncIndex);
			}
			break;

			default:
				break;
			}
		}
		return false;
	}
}
