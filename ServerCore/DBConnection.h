#pragma once

#include <sql.h>
#include <sqlext.h>

/* -----------------
	DBConnection
----------------- */

class DBConnection
{
public:
	bool	Connect(SQLHENV henv, const WCHAR* connectionString);
	void	Clear();

	bool	Execute(const WCHAR* query);		// 쿼리문
	bool	Fetch();							// 결과를 받기위한 용도
	int32	GetRowCount();					// 데이터가 몇개있는지
	void	Unbind();

public:
	bool	BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
	bool	BindCol(SQLUSMALLINT columIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);		// 데이터를 받아올때
	void	HandleError(SQLRETURN ret);

private:
	SQLHDBC		_connection = SQL_NULL_HANDLE;
	SQLHSTMT	_statement = SQL_NULL_HANDLE;

};

