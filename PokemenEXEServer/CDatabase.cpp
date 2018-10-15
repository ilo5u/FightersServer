#include "stdafx.h"
#include "CDatabase.h"

CDatabase::CDatabase() :
    m_mysql()
{
    mysql_init(&m_mysql);
}

CDatabase::~CDatabase()
{
    mysql_close(&m_mysql);
}

bool CDatabase::Connect(const std::string& user, const std::string& password, const std::string& database)
{
    if (mysql_real_connect(&m_mysql, "localhost",
                           user.c_str(), password.c_str(), database.c_str(),
                           0, NULL, 0))
    {
        return false;
    }
    return true;
}

std::string CDatabase::Select(const std::string& query, int valueCnt)
{
    MYSQL_RES *sqlResult = NULL;
    MYSQL_ROW sqlRow;

    std::string queryResult;
    int iRetVal = mysql_query(&m_mysql, query.c_str());
    if (!iRetVal)
    {
		sqlResult = mysql_store_result(&m_mysql);
		if (sqlResult)
		{
			while (sqlRow = mysql_fetch_row(sqlResult))
			{
				queryResult.clear();
				for (int i = 0; i < valueCnt; ++i)
					queryResult += sqlRow[i] + std::string{ "\n" };
			}
		}

    }

    if (sqlResult != NULL)
        mysql_free_result(sqlResult);
    sqlResult = NULL;

    return queryResult;
}

