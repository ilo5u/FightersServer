#include "stdafx.h"

namespace Pokemen
{
	/// <summary>
	/// 
	/// </summary>
	PokemenProperty::PokemenProperty(std::string name, int16_t hitpoints, int16_t attack, int16_t defense, int16_t agility, PokemenType type) :
		m_name(name),
		m_hpoints(hitpoints),
		m_attack(attack),
		m_defense(defense),
		m_agility(agility),
		m_type(type),
		m_level(1),
		m_exp(0)
	{
		m_break = BreakValueCalculator(break_base, m_agility, m_attack);
		m_critical = CriticalValueCalculator(critical_base, m_agility);
		m_hitratio = HitratioValueCalculator(hitratio_base, m_agility, m_hpoints);
		m_parryratio = ParryratioValueCalculator(parryratio_base, m_agility, m_defense);
	}

	/// <summary>
	/// 
	/// </summary>
	PokemenProperty::~PokemenProperty()
	{
	}

	std::string PokemenProperty::GetName() const
	{
		return m_name;
	}

	int16_t PokemenProperty::GetHpoints() const
	{
		return m_hpoints;
	}

	int16_t PokemenProperty::GetAttack() const
	{
		return m_attack;
	}

	int16_t PokemenProperty::GetDefense() const
	{
		return m_defense;
	}

	int16_t PokemenProperty::GetAgility() const
	{
		return m_agility;
	}

	int16_t PokemenProperty::GetBreak() const
	{
		return m_break;
	}

	int16_t PokemenProperty::GetCritical() const
	{
		return m_critical;
	}

	int16_t PokemenProperty::GetHitratio() const
	{
		return m_hitratio;
	}

	int16_t PokemenProperty::GetParryratio() const
	{
		return m_parryratio;
	}

	PokemenType PokemenProperty::GetType() const
	{
		return m_type;
	}

	int16_t PokemenProperty::GetLevel() const
	{
		return m_level;
	}

	int16_t PokemenProperty::GetExp() const
	{
		return m_exp;
	}

	bool PokemenProperty::SetName(const std::string& name)
	{
		m_name = name;
		return false;
	}

	bool PokemenProperty::SetHpoints(int16_t hpoints)
	{
		m_hpoints = hpoints;
		return false;
	}

	bool PokemenProperty::SetAttack(int16_t attack)
	{
		m_attack = attack;
		return false;
	}

	bool PokemenProperty::SetDefense(int16_t defense)
	{
		m_defense = defense;
		return false;
	}

	bool PokemenProperty::SetAgility(int16_t agility)
	{
		m_agility = agility;
		return false;
	}

	bool PokemenProperty::SetBreak(int16_t brea)
	{
		m_break = brea;
		return false;
	}

	bool PokemenProperty::SetCritical(int16_t critical)
	{
		m_critical = critical;
		return false;
	}

	bool PokemenProperty::SetHitratio(int16_t hitratio)
	{
		m_hitratio = hitratio;
		return false;
	}

	bool PokemenProperty::SetParryratio(int16_t parryratio)
	{
		m_parryratio = parryratio;
		return false;
	}

	bool PokemenProperty::SetLevel(int16_t level)
	{
		m_level = level;
		return false;
	}

	bool PokemenProperty::SetExp(int16_t exp)
	{
		m_exp = exp;
		return false;
	}

	/// <summary>
/// 
/// </summary>
	int16_t PokemenProperty::BreakValueCalculator(int16_t base, int16_t primary_affect, int16_t secondary_affect)
	{
		return base - (int16_t)100 * (int16_t)std::sqrt((double)primary_affect) + (int16_t)100 * (int16_t)std::sqrt((double)secondary_affect);
	}

	/// <summary>
	/// 
	/// </summary>
	int16_t PokemenProperty::CriticalValueCalculator(int16_t base, int16_t affect)
	{
		return base + (int16_t)4 * (int16_t)std::sqrt((double)affect);
	}

	/// <summary>
	/// 
	/// </summary>
	int16_t PokemenProperty::HitratioValueCalculator(int16_t base, int16_t primary_affect, int16_t secondary_affect)
	{
		return base - (int16_t)4 * (int16_t)std::sqrt((double)primary_affect) + (int16_t)std::sqrt(std::sqrt((double)secondary_affect));
	}

	/// <summary>
	/// 
	/// </summary>
	int16_t PokemenProperty::ParryratioValueCalculator(int16_t base, int16_t primary_affect, int16_t secondary_affect)
	{
		return base + (int16_t)4 * (int16_t)std::sqrt((double)primary_affect) - (int16_t)std::sqrt((double)secondary_affect);
	}

	/// <summary>
	/// 
	/// </summary>
	int16_t PokemenProperty::AttackDamageCalculator(int16_t primary_affect, int16_t secondary_affect)
	{
		return static_cast<int16_t>(((double)primary_affect * (double)primary_affect) / ((double)primary_affect + (double)secondary_affect));
	}

	/// <summary>
	/// 
	/// </summary>
	int16_t PokemenProperty::HealingHpointsCalculator(int16_t primary_affect, int16_t secondary_affect)
	{
		return static_cast<int16_t>((double)primary_affect * ((double)secondary_affect / 200));
	}

	/// <summary>
	/// 
	/// </summary>
	int16_t PokemenProperty::BloodingDamageCalculator(int16_t primary_affect, int16_t secondary_affect)
	{
		return static_cast<int16_t>(((double)primary_affect * 10.0) / std::sqrt((double)secondary_affect));
	}

	bool BasePlayer::InState(BasePlayer::State inState) const
	{
		return (static_cast<uint16_t>(this->m_state) & static_cast<uint16_t>(inState)) == 0 ? false : true;
	}

	bool BasePlayer::AddState(BasePlayer::State newState)
	{
		this->m_state = static_cast<BasePlayer::State>(static_cast<uint16_t>(this->m_state) | static_cast<uint16_t>(newState));
		return false;
	}

	bool BasePlayer::SubState(BasePlayer::State oldState)
	{
		this->m_state = static_cast<BasePlayer::State>(static_cast<uint16_t>(this->m_state) & (~static_cast<uint16_t>(oldState)));
		return false;
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::BasePlayer(std::string name, int16_t hitpoints, int16_t attack, int16_t defense, int16_t agility, PokemenType type) :
		PokemenProperty(name, hitpoints, attack, defense, agility, type),
		m_state(State::NORMAL),
		m_anger(0),
		m_hpoints_limitation(hitpoints)
	{
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::~BasePlayer()
	{
	}

	/// <summary>
	/// 
	/// </summary>
	std::string BasePlayer::Attack(HCPokemenPlayer oppoent)
	{
		throw std::exception("Not implement class in method Attack()");
		return false;
	}

	/// <summary>
	/// 
	/// </summary>
	int16_t BasePlayer::IsAttacked(int16_t damage)
	{
		throw std::exception("Method IsAttacked() in CPokemenPlayer is not implement.");
	}

	/// <summary>
	/// 
	/// </summary>
	bool BasePlayer::SetPrimarySkill(const SkillBase& skill)
	{
		throw std::exception("Method IsAttacked() in CPokemenPlayer is not implement.");
	}

	bool BasePlayer::SetAnger(int anger)
	{
		this->m_anger = anger;
		return false;
	}

	bool BasePlayer::Update(int extra)
	{
		this->m_exp = min(this->m_exp + extra, level_limitation * exp_base - 1);

		int16_t oldlevel = this->m_level;
		this->m_level = this->m_exp / exp_base + 1;
		if (this->m_level > oldlevel)
		{
			_level_up_property_distribution();
			return true;
		}
		return false;
	}

	/// <summary>
	/// 
	/// </summary>
	BasePlayer::State BasePlayer::GetState() const
	{
		return this->m_state;
	}

	int16_t BasePlayer::GetAnger() const
	{
		return this->m_anger;
	}

	void BasePlayer::_level_up_property_distribution()
	{
	}
}