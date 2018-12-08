#pragma once

namespace Pokemen
{
	typedef std::string String;
	typedef std::mutex  Mutex;
	typedef std::thread Thread;

	/// <summary>
	/// 
	/// </summary>
	class Pokemen
	{
	public:
		Pokemen(PokemenType type, int level, Id id = 0);
		Pokemen(const Property& prop, int career);
		Pokemen(const Pokemen& other);
		Pokemen(Pokemen&& other);
		Pokemen& operator=(const Pokemen& other);
		Pokemen& operator=(Pokemen&& other);
		virtual ~Pokemen();

	public:
		// 获取属性值
		int GetId() const;
		PokemenType GetType() const;
		String GetName() const;

		int GetHpoints() const;
		int GetAttack() const;
		int GetDefense() const;
		int GetAgility() const;

		int GetInterval() const;
		int GetCritical() const;
		int GetHitratio() const;
		int GetParryratio() const;

		int GetCareer() const;
		int GetLevel() const;
		int GetExp() const;

	public:
		bool Promote(int career);
		void RenewProperty(const ::Pokemen::Property& prop, int carrer);

	private:
		PBasePlayer m_instance;
	};
}