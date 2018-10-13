#pragma once

#include <vector>

class CDatabase;
typedef CDatabase * HDATABASE;


class CUserManager;
typedef CUserManager * HUSERMANAGER;

typedef typename std::vector<HUSERMANAGER> USER_LIST;

class CServer
{
public:

private:
	HDATABASE m_hDatabase;
	USER_LIST m_user_list;
};