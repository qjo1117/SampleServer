#include "pch.h"
#include "DBConnection.h"

bool DBConnection::Connect(SQLHENV henv, const WCHAR* connectionString)
{
    // ó���� Pool���� ȣ���ϴ� �༮

    if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection) != SQL_SUCCESS) {
        return false;
    }
    
    WCHAR stringBuffer[MAX_PATH] = { 0 };
    ::wcscpy_s(stringBuffer, connectionString);

    WCHAR resultString[MAX_PATH] = { 0 };
    SQLSMALLINT resultStringLen = 0;

    // Driver�� �̿��ؼ� Connect���ش�.
    SQLRETURN ret = ::SQLDriverConnectW(
        _connection,
        nullptr,
        reinterpret_cast<SQLWCHAR*>(stringBuffer),
        _countof(stringBuffer),
        OUT reinterpret_cast<SQLWCHAR*>(resultString),
        _countof(resultString),
        OUT & resultStringLen,
        SQL_DRIVER_PROMPT
    );

    // _connection������ ����� statement�� �Ҵ�������
    if (::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement) != SQL_SUCCESS) {
        return false;
    }

    return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
}

void DBConnection::Clear()
{
    if (_connection != SQL_NULL_HANDLE) {
        ::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
        _connection = SQL_NULL_HANDLE;
    }
    if (_statement != SQL_NULL_HANDLE) {
        ::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
        _statement = SQL_NULL_HANDLE;
    }
}

bool DBConnection::Execute(const WCHAR* query)
{
    SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTSL);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        return true;
    }

    HandleError(ret);       // �α� ���
    return false;
}

bool DBConnection::Fetch()
{
    SQLRETURN ret = ::SQLFetch(_statement);

    switch (ret) {
    case SQL_SUCCESS:
    case SQL_SUCCESS_WITH_INFO: 
        return true;

    case SQL_NO_DATA: 
        return false; 

    case SQL_ERROR:
        HandleError(ret);
        return false;

    default:
        return true;
    }

    return false;
}

int32 DBConnection::GetRowCount()
{
    SQLLEN count = 0;
    SQLRETURN ret = ::SQLRowCount(_statement, OUT & count);

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        return static_cast<int32>(count);
    }

    return -1;
}

void DBConnection::Unbind()
{
    // �̰� ����ϴ� ������ ���� �����͸� ����ϱ� ����
    // ���� �ִ� �����Ͱ� ���ε� ���ɼ��� �����Ƿ� �����͸� ��������Ѵ�.

    // �ʱ�ȭ
    ::SQLFreeStmt(_statement, SQL_UNBIND);
    ::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
    ::SQLFreeStmt(_statement, SQL_CLOSE);
}

bool DBConnection::BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index)
{
    SQLRETURN ret = ::SQLBindParameter(_statement, paramIndex, SQL_PARAM_INPUT, cType, sqlType, len, 0, ptr, 0, index);
    
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleError(ret);
        return false;
    }

    return true;
}

bool DBConnection::BindCol(SQLUSMALLINT columIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index)
{
    SQLRETURN ret = ::SQLBindCol(_statement, columIndex, cType, value, len, index);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        HandleError(ret);
        return false;
    }

    return true;
}

void DBConnection::HandleError(SQLRETURN ret)
{
    if (ret == SQL_SUCCESS) {           // �� �����ߴµ� ����ߴ°�
        return;
    }

    SQLSMALLINT index = 1;
    SQLWCHAR sqlState[MAX_PATH] = { 0 };
    SQLINTEGER nativeErr = 0;
    SQLWCHAR errMsg[MAX_PATH] = { 0 };
    SQLSMALLINT msgLen = 0;
    SQLRETURN errorRet = 0;

    while (true) {
        errorRet = ::SQLGetDiagRecW(
            SQL_HANDLE_STMT,
            _statement,
            index,
            sqlState,
            OUT & nativeErr,
            errMsg,
            _countof(errMsg),
            OUT & msgLen
        );

        if (errorRet == SQL_NO_DATA) {
            break;
        }

        if (errorRet != SQL_SUCCESS && errorRet != SQL_SUCCESS_WITH_INFO) {
            break;
        }

        // TOOD : Log
        wcout.imbue(locale("kor"));
        wcout << errMsg << endl;            // ���� ���������
        // Live���� ������ ��ﶧ�� �ַܼ� �����ʰ� ���񽺷� ������ ����.

        index += 1;
    }

}
