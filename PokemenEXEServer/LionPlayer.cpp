#include "stdafx.h"

namespace Pokemen
{
	static std::vector<std::string> g_Strengthname_Base{ "Charizard", "Squirtle", "Wartortle", "Blastoise" };

	/// <summary>
	/// 
	/// </summary>
	Lion::Lion(Skill::Type primary_skill, int level) :
		BasePlayer(
			g_Strengthname_Base[_Random(g_Strengthname_Base.size())],
			hitpoints_base + _Random(property_offset),
			attack_base + _Random(property_offset),
			defense_base + _Random(property_offset),
			agility_base + _Random(property_offset),
			PokemenType::LION
		),
		m_skill(primary_skill)
	{
		while (this->m_level < min(level, level_limitation))
			Update(exp_base);
	}

	/// <summary>
	/// 
	/// </summary>
	Lion::~Lion()
	{
	}

	Lion::Skill::Skill(Type primary_skill)
	{
		m_primary_skill = primary_skill;
	}

	/// <summary>
	/// 
	/// </summary>
	std::string Lion::Attack(BasePlayer& opponent)
	{
		return false;
	}

	int16_t Lion::IsAttacked(int16_t damage)
	{
		return int16_t();
	}

	bool Lion::SetPrimarySkill(Skill::Type skill)
	{
		return false;
	}
}