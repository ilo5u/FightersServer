#pragma once
#include <mysql.h>

class CDatabase
{
public:
    CDatabase();
    ~CDatabase();
public:
        bool Connect(const std::string& user, const std::string& password, const std::string& database);
        std::vector<std::string> Select(const std::string& query, int valueCnt);

private:
	MYSQL m_mysql;
};
typedef CDatabase * HDATABASE;
