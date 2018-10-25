#include "stdafx.h"

namespace Pokemen
{
	static const Strings NamesOfHedgehog{ "Caterpie", "Metapod", "Butterfree", "Weedle" };

	/// <summary>
	/// 
	/// </summary>
	Hedgehog::Hedgehog(Skill::Type primarySkill, int level) :
		BasePlayer(
			NamesOfHedgehog[_Random(NamesOfHedgehog.size())],
			hitpointsBase + _Random(propertyOffset),
			attackBase + _Random(propertyOffset),
			defenseBase + _Random(propertyOffset),
			agilityBase + _Random(propertyOffset),
			PokemenType::HEDGEHOG
		),
		m_skill(primarySkill)
	{
		while (this->m_level < std::min<Value>(level, levelLimitation))
			Update(expBase);
	}

	/// <summary>
	/// 
	/// </summary>
	Hedgehog::~Hedgehog()
	{
	}

	Hedgehog::Skill::Skill(Type primarySkill)
	{
		m_primarySkill = primarySkill;
	}

	String Hedgehog::Attack(BasePlayer& opponent)
	{
		return { };
	}

	Value Hedgehog::IsAttacked(Value damage)
	{
		return Value();
	}

	bool Hedgehog::SetPrimarySkill(Skill::Type primary_skill)
	{
		return false;
	}

}
