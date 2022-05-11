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

	bool	Execute(const WCHAR* query);		// ������
	bool	Fetch();							// ����� �ޱ����� �뵵
	int32	GetRowCount();					// �����Ͱ� ��ִ���
	void	Unbind();

public:
	bool	BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
	bool	BindCol(SQLUSMALLINT columIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);		// �����͸� �޾ƿö�
	void	HandleError(SQLRETURN ret);

private:
	SQLHDBC		_connection = SQL_NULL_HANDLE;
	SQLHSTMT	_statement = SQL_NULL_HANDLE;

};

