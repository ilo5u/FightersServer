#pragma once
#include <mysql.h>

class CDatabase
{
public:
	bool Connect();

private:
	MYSQL m_mysql;
};
typedef CDatabase * HDATABASE;