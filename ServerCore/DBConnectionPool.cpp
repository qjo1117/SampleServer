#include "pch.h"
#include "DBConnectionPool.h"

/* -----------------------
    DBConnectionPool
----------------------- */

DBConnectionPool::DBConnectionPool()
{
}

DBConnectionPool::~DBConnectionPool()
{
    Clear();
}

bool DBConnectionPool::Connect(int32 connectionCount, const WCHAR* connectionString)
{
    WRITE_LOCK;

    if (::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_enviroment) != SQL_SUCCESS) {
        return false;
    }

    if (::SQLSetEnvAttr(_enviroment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0) != SQL_SUCCESS) {
        return false;
    }

    // DBConnection객체를 이용해서 연결을 요청할텐데 다수의 쓰레드, 객체가 존재하기 때문에
    for (int32 i = 0; i < connectionCount; ++i) {
        DBConnection* connection = xnew<DBConnection>();
        if (connection->Connect(_enviroment, connectionString) == false) {
            return false;
        }
        
        _connections.push_back(connection);
    }

    return true;
}

void DBConnectionPool::Clear()
{
    WRITE_LOCK;

    if (_enviroment != SQL_NULL_HANDLE) {
        ::SQLFreeHandle(SQL_HANDLE_ENV, _enviroment);
        _enviroment = SQL_NULL_HANDLE;
    }

    for (DBConnection* connection : _connections) {
        xdelete(connection);
    }

    _connections.clear(); 
}

DBConnection* DBConnectionPool::Pop()
{
    WRITE_LOCK;

    if (_connections.empty()) {
        return nullptr;
    }

    DBConnection* connection = _connections.back();
    _connections.pop_back();

    return connection;
}

void DBConnectionPool::Push(DBConnection* connection)
{
    WRITE_LOCK;
    _connections.push_back(connection);
}
