#include "stdafx.h"

namespace Pokemen
{
	static std::vector<std::string> g_Agilename_Base{ "Kakuna", "Beedrill", "Pidgey", "Pidgeotto" };

	/// <summary>
	/// 
	/// </summary>
	Eagle::Eagle(Skill::Type primary_skill, int level) :
		BasePlayer(
			g_Agilename_Base[_Random(g_Agilename_Base.size())],
			hitpoints_base + _Random(property_offset),
			attack_base + _Random(property_offset),
			defense_base + _Random(property_offset),
			agility_base + _Random(property_offset),
			PokemenType::EAGLE
		),
		m_skill(primary_skill)
	{
		while (this->m_level < min(level, level_limitation))
			Update(exp_base);
	}

	/// <summary>
	/// 
	/// </summary>
	Eagle::~Eagle()
	{
	}

	Eagle::Skill::Skill(Type primary_skill)
	{
		m_primary_skill = primary_skill;
	}

	std::string Eagle::Attack(BasePlayer& opponent)
	{
		return false;
	}

	int16_t Eagle::IsAttacked(int16_t damage)
	{
		return int16_t();
	}

	bool Eagle::SetPrimarySkill(Skill::Type primary_skill)
	{
		return false;
	}
}
