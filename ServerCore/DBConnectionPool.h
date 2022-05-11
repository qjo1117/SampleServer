#pragma once

#include "DBConnection.h"

/* -----------------------
	DBConnectionPool
----------------------- */

class DBConnectionPool
{

public:
	DBConnectionPool();
	~DBConnectionPool();

	bool Connect(int32 connectionCount, const WCHAR* connectionString);
	void Clear();

	// Ref까지 할 필요는 없어서 예외적으로 사용
	DBConnection* Pop();
	void Push(DBConnection* connection);

private:
	USE_LOCK;
	SQLHENV					_enviroment = SQL_NULL_HANDLE;		// 환경 핸들
	Vector<DBConnection*>	_connections;
	
};

