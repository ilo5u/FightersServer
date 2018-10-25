#include "stdafx.h"

namespace Pokemen
{
	static const Strings NamesOfEagle{ "Kakuna", "Beedrill", "Pidgey", "Pidgeotto" };

	/// <summary>
	/// 
	/// </summary>
	Eagle::Eagle(Skill::Type primarySkill, int level) :
		BasePlayer(
			NamesOfEagle[_Random(NamesOfEagle.size())],
			hitpointsBase + _Random(propertyOffset),
			attackBase + _Random(propertyOffset),
			defenseBase + _Random(propertyOffset),
			agilityBase + _Random(propertyOffset),
			PokemenType::EAGLE
		),
		m_skill(primarySkill)
	{
		while (this->m_level < std::min<Value>(level, levelLimitation))
			Update(expBase);
	}

	/// <summary>
	/// 
	/// </summary>
	Eagle::~Eagle()
	{
	}

	Eagle::Skill::Skill(Type primarySkill)
	{
		m_primarySkill = primarySkill;
	}

	String Eagle::Attack(BasePlayer& opponent)
	{
		return {};
	}

	Value Eagle::IsAttacked(Value damage)
	{
		return Value();
	}

	bool Eagle::SetPrimarySkill(Skill::Type primarySkill)
	{
		return false;
	}
}
