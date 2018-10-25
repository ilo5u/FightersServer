#include "stdafx.h"

namespace Pokemen
{
	static const Strings NamesOfLion{ "Charizard", "Squirtle", "Wartortle", "Blastoise" };

	/// <summary>
	/// 
	/// </summary>
	Lion::Lion(Skill::Type primarySkill, int level) :
		BasePlayer(
			NamesOfLion[_Random(NamesOfLion.size())],
			hitpointsBase + _Random(propertyOffset),
			attackBase + _Random(propertyOffset),
			defenseBase + _Random(propertyOffset),
			agilityBase + _Random(propertyOffset),
			PokemenType::LION
		),
		m_skill(primarySkill)
	{
		while (this->m_level < std::min<Value>(level, levelLimitation))
			Update(expBase);
	}

	/// <summary>
	/// 
	/// </summary>
	Lion::~Lion()
	{
	}

	Lion::Skill::Skill(Type primarySkill)
	{
		m_primarySkill = primarySkill;
	}

	/// <summary>
	/// 
	/// </summary>
	String Lion::Attack(BasePlayer& opponent)
	{
		return { };
	}

	Value Lion::IsAttacked(Value damage)
	{
		return Value();
	}

	bool Lion::SetPrimarySkill(Skill::Type primarySkill)
	{
		return false;
	}
}