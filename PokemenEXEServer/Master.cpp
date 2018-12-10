#include "stdafx.h"

namespace Pokemen
{
	static const Strings namesOfMaster{ "Bulbasaur", "Ivysaur", "Venusaur", "Charmander" };

	Master::Career::Career(Career::Type type) :
		type(type)
	{
	}

	Master::Master(int level) :
		BasePlayer(
			PokemenType::MASTER, namesOfMaster[_Random(namesOfMaster.size())],
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

	Master::Master(const Property& prop, Career::Type career) :
		BasePlayer(prop),
		m_career(career)
	{
	}

	Master::~Master()
	{
	}

	Master::Career::Type Master::GetCareer() const
	{
		return this->m_career.type;
	}

	bool Master::Promote(Career::Type career)
	{
		if (this->m_career.type == Career::Type::Normal)
		{
			this->m_career.type = career;
			int bonus = (this->m_property.m_level - 8) * 10 + 100;
			switch (this->m_career.type)
			{
			case Career::Type::GreatMasterOfLight:
			{
				this->m_property.m_attack += ConvertValueByPercent(this->m_property.m_attack, Career::Lighter::damageDecIndex);
				this->m_property.m_hpoints += ConvertValueByPercent(ConvertValueByPercent(this->m_property.m_hpoints, Career::Lighter::hpointsIncIndex), bonus);
			}
			break;

			case Career::Type::GreatMasterOfDark:
			{
				this->m_property.m_defense += ConvertValueByPercent(this->m_property.m_defense, Career::Darker::defenseDecIndex);
				this->m_property.m_attack += ConvertValueByPercent(ConvertValueByPercent(this->m_property.m_attack, Career::Darker::damageIncIndex), bonus);
			}
			break;

			default:
				break;
			}
		}
		return false;
	}
}