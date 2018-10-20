#include "stdafx.h"

namespace Pokemen
{
	static std::vector<std::string> g_Defensivename_Base{ "Caterpie", "Metapod", "Butterfree", "Weedle" };

	/// <summary>
	/// 
	/// </summary>
	Hedgehog::Hedgehog(Skill::Type primary_skill, int level) :
		BasePlayer(
			g_Defensivename_Base[_Random(g_Defensivename_Base.size())],
			hitpoints_base + _Random(property_offset),
			attack_base + _Random(property_offset),
			defense_base + _Random(property_offset),
			agility_base + _Random(property_offset),
			PokemenType::HEDGEHOG
		),
		m_skill(primary_skill)
	{
		while (this->m_level < min(level, level_limitation))
			Update(exp_base);
	}

	/// <summary>
	/// 
	/// </summary>
	Hedgehog::~Hedgehog()
	{
	}

	Hedgehog::Skill::Skill(Type primary_skill)
	{
		m_primary_skill = primary_skill;
	}

	std::string Hedgehog::Attack(BasePlayer& opponent)
	{
		return false;
	}

	int16_t Hedgehog::IsAttacked(int16_t damage)
	{
		return int16_t();
	}

	bool Hedgehog::SetPrimarySkill(Skill::Type primary_skill)
	{
		return false;
	}

}
