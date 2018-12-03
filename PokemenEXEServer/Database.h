#pragma once
#include <mysql.h>

class Database
{
public:
    Database();
    ~Database();
public:
    bool Connect(const std::string& user, const std::string& password, const std::string& database);
	void Disconnect();
	bool Insert(const std::string& query);
	bool Update(const std::string& query);
    std::vector<std::string> Select(const std::string& query, int valueCnt);

private:
	std::mutex m_locker;
	MYSQL m_mysql;
};
typedef Database * HDATABASE;
