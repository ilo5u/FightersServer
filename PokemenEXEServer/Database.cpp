#include "stdafx.h"
#include "Database.h"

Database::Database() :
    m_mysql()
{
    mysql_init(&m_mysql);
}

Database::~Database()
{
    mysql_close(&m_mysql);
}

bool Database::Connect(const std::string& user, const std::string& password, const std::string& database)
{
    if (mysql_real_connect(&m_mysql, "localhost",
                           user.c_str(), password.c_str(), database.c_str(),
                           0, NULL, 0))
    {
        return true;
    }
    return false;
}

bool Database::Insert(const std::string& query)
{
	if (!mysql_query(&m_mysql, query.c_str()))
		return true;
	return false;
}

bool Database::Update(const std::string& query)
{
	if (!mysql_query(&m_mysql, query.c_str()))
		return true;
	return false;
}

std::vector<std::string> Database::Select(const std::string& query, int valueCnt)
{
    MYSQL_RES *sqlResult = NULL;
    MYSQL_ROW sqlRow;

    std::vector<std::string> queryResult;
	std::string rowResult;
    int iRetVal = mysql_query(&m_mysql, query.c_str());
    if (!iRetVal)
    {
		sqlResult = mysql_store_result(&m_mysql);
		if (sqlResult)
		{
			while (sqlRow = mysql_fetch_row(sqlResult))
			{
				rowResult.clear();
				for (int i = 0; i < valueCnt; ++i)
					rowResult += sqlRow[i] + std::string{ "\n" };
				queryResult.push_back(rowResult);
			}
		}
    }

    if (sqlResult != NULL)
        mysql_free_result(sqlResult);
    sqlResult = NULL;

    return queryResult;
}

