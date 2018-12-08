#include "stdafx.h"

namespace Pokemen
{
	/// <summary>
	/// 
	/// </summary>
	Pokemen::Pokemen(PokemenType type, int level, Id id) :
		m_instance(nullptr)
	{
		if (type == PokemenType::DEFAULT)
			type = static_cast<PokemenType>(_Random(1, 4));

		switch (type)
		{
		case PokemenType::MASTER:
			this->m_instance = new Master{ level };
			break;

		case PokemenType::KNIGHT:
			this->m_instance = new Knight{ level };
			break;

		case PokemenType::GUARDIAN:
			this->m_instance = new Guardian{ level };
			break;

		default:
			this->m_instance = new Assassin{ level };
			break;
		}
	}

	Pokemen::Pokemen(const Property& prop, int career) :
		m_instance(nullptr)
	{
		if (prop.m_type == PokemenType::DEFAULT 
			|| (career < 0 || career > 2))
			throw std::exception("Create a player pokemen with a error type.");

		switch (prop.m_type)
		{
		case PokemenType::MASTER:
			this->m_instance = new Master{ prop, static_cast<Master::Career::Type>(career) };
			break;

		case PokemenType::KNIGHT:
			this->m_instance = new Knight{ prop, static_cast<Knight::Career::Type>(career) };
			break;

		case PokemenType::GUARDIAN:
			this->m_instance = new Guardian{ prop, static_cast<Guardian::Career::Type>(career) };
			break;

		case PokemenType::ASSASSIN:
			this->m_instance = new Assassin{ prop, static_cast<Assassin::Career::Type>(career) };
			break;

		default:
			throw std::exception("Create a player pokemen with a error type.");
			break;
		}
	}

	Pokemen::Pokemen(const Pokemen& other)
	{
		switch (other.m_instance->GetType())
		{
		case PokemenType::MASTER:
			this->m_instance = new Master{ *static_cast<PMaster>(other.m_instance) };
			break;

		case PokemenType::KNIGHT:
			this->m_instance = new Knight{ *static_cast<PKnight>(other.m_instance) };
			break;

		case PokemenType::GUARDIAN:
			this->m_instance = new Guardian{ *static_cast<PGuardian>(other.m_instance) };
			break;

		case PokemenType::ASSASSIN:
			this->m_instance = new Assassin{ *static_cast<PAssassin>(other.m_instance) };
			break;

		default:
			throw std::exception("Create a player pokemen with a error type.");
			break;
		}
	}

	Pokemen::Pokemen(Pokemen&& other)
	{
		m_instance = other.m_instance;
		other.m_instance = nullptr;
	}

	Pokemen& Pokemen::operator=(const Pokemen& other)
	{
		switch (other.m_instance->GetType())
		{
		case PokemenType::MASTER:
			this->m_instance = new Master{ *static_cast<PMaster>(other.m_instance) };
			break;

		case PokemenType::KNIGHT:
			this->m_instance = new Knight{ *static_cast<PKnight>(other.m_instance) };
			break;

		case PokemenType::GUARDIAN:
			this->m_instance = new Guardian{ *static_cast<PGuardian>(other.m_instance) };
			break;

		case PokemenType::ASSASSIN: 
			this->m_instance = new Assassin{ *static_cast<PAssassin>(other.m_instance) };
			break;

		default:
			throw std::exception("Create a player pokemen with a error type.");
			break;
		}
		return *this;
	}

	Pokemen& Pokemen::operator=(Pokemen&& other)
	{
		this->m_instance = other.m_instance;
		other.m_instance = nullptr;
		return *this;
	}

	/// <summary>
	/// 
	/// </summary>
	Pokemen::~Pokemen()
	{
		delete this->m_instance;
		this->m_instance = nullptr;
	}

	int Pokemen::GetId() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetId();
	}

	/// <summary>
	/// 
	/// </summary>
	String Pokemen::GetName() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetName();
	}

	/// <summary>
	/// 
	/// </summary>
	int Pokemen::GetHpoints() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetHpoints();
	}

	/// <summary>
	/// 
	/// </summary>
	int Pokemen::GetAttack() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetAttack();
	}

	/// <summary>
	/// 
	/// </summary>
	int Pokemen::GetDefense() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetDefense();
	}

	/// <summary>
	/// 
	/// </summary>
	int Pokemen::GetAgility() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetAgility();
	}

	/// <summary>
	/// 
	/// </summary>
	int Pokemen::GetInterval() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetInterval();
	}

	/// <summary>
	/// 
	/// </summary>
	int Pokemen::GetCritical() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetCritical();
	}

	/// <summary>
	/// 
	/// </summary>
	int Pokemen::GetHitratio() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetHitratio();
	}

	/// <summary>
	/// 
	/// </summary>
	int Pokemen::GetParryratio() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetParryratio();
	}

	int Pokemen::GetAnger() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetAnger();
	}

	int Pokemen::GetCareer() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
		{
			switch (this->m_instance->GetType())
			{
			case PokemenType::MASTER:
				return static_cast<int>((static_cast<PMaster>(this->m_instance))->GetCareer());

			case PokemenType::KNIGHT:
				return static_cast<int>((static_cast<PKnight>(this->m_instance))->GetCareer());

			case PokemenType::GUARDIAN:
				return static_cast<int>((static_cast<PGuardian>(this->m_instance))->GetCareer());

			case PokemenType::ASSASSIN:
				return static_cast<int>((static_cast<PAssassin>(this->m_instance))->GetCareer());

			default:
				throw std::exception("CPokemenManager is not implement.");
			}
		}
	}

	int Pokemen::GetLevel() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetLevel();
	}

	int Pokemen::GetExp() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetExp();
	}

	bool Pokemen::Upgrade(int exp)
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->Upgrade(exp);
	}

	bool Pokemen::SetPrimarySkill(int skill)
	{
		if (skill < 0 || skill > 2)
			return false;
		switch (this->m_instance->GetType())
		{
		case PokemenType::MASTER:
			static_cast<PMaster>(this->m_instance)->SetPrimarySkill(static_cast<Master::Skill::Type>(skill));
			break;

		case PokemenType::KNIGHT:
			static_cast<PKnight>(this->m_instance)->SetPrimarySkill(static_cast<Knight::Skill::Type>(skill));
			break;

		case PokemenType::GUARDIAN:
			static_cast<PGuardian>(this->m_instance)->SetPrimarySkill(static_cast<Guardian::Skill::Type>(skill));
			break;

		case PokemenType::ASSASSIN:
			static_cast<PAssassin>(this->m_instance)->SetPrimarySkill(static_cast<Assassin::Skill::Type>(skill));
			break;

		default:
			break;
		}
		return true;
	}

	bool Pokemen::Promote(int career)
	{
		if (career < 1 || career > 2)
			return false;
		switch (this->m_instance->GetType())
		{
		case PokemenType::MASTER:
			static_cast<PMaster>(this->m_instance)->Promote(static_cast<Master::Career::Type>(career));
			break;

		case PokemenType::KNIGHT:
			static_cast<PKnight>(this->m_instance)->Promote(static_cast<Knight::Career::Type>(career));
			break;

		case PokemenType::GUARDIAN:
			static_cast<PGuardian>(this->m_instance)->Promote(static_cast<Guardian::Career::Type>(career));
			break;

		case PokemenType::ASSASSIN:
			static_cast<PAssassin>(this->m_instance)->Promote(static_cast<Assassin::Career::Type>(career));
			break;

		default:
			break;
		}
		return true;
	}

	void Pokemen::RenewProperty(const::Pokemen::Property& prop, int carrer)
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
		{
			this->m_instance->SetProperty(prop);
		}
	}

	PokemenType Pokemen::GetType() const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->GetType();
	}

	void Pokemen::SetMaxHpoints()
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			this->m_instance->SetMaxHpoints();
	}

	bool Pokemen::InState(BasePlayer::State nowState) const
	{
		if (this->m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return this->m_instance->InState(nowState);
	}

	/// <summary>
	/// 
	/// </summary>
	std::string Pokemen::Attack(Pokemen& opponent)
	{
		if (this->m_instance == nullptr || opponent.m_instance == nullptr)
			throw std::exception("CPokemenManager is not implement.");
		else
			return static_cast<PBasePlayer>(this->m_instance)->Attack(*static_cast<PBasePlayer>(opponent.m_instance));
	}
}