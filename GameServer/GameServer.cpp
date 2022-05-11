#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include <tchar.h>
#include "Protocol.pb.h"
#include "Job.h"
#include "Room.h"
#include "Player.h"

#include "DBConnectionPool.h"

enum
{
	WORKER_TICK = 64
};

void DoWorkerJob(ServerServiceRef& service)
{
	while (true)
	{
		LEndTickCount = ::GetTickCount64() + WORKER_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직까지 (패킷 핸들러에 의해)
		service->GetIocpCore()->Dispatch(10);

		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐
		ThreadManager::DoGlobalQueueWork();
	}
}

int main()
{
	// 서버테스트
	// -------------------------------------------------
	
	// LIVE에서는 STRING에 해당하는 녀석들을 따로 파일, 데이터등으로 저장해서 연결할 수 잇도록 하는 게 좋다고 함

	// MYSQL로 사용할 꺼면 mysql로 연결된 odbc를 설치를 해야한다.
	ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={SQL Server Native Client 11.0};Server=(localdb)\\MSSQLLocalDB;Database=ServerDB;Trusted_Connection=Yes;"));

	// Create Table
	{
		auto query = L"										\
			DROP TABLE IF EXISTS[dbo].[Gold];				\
			CREATE TABLE [dbo].[Gold]						\
			(												\
				[id] INT NOT NULL PRIMARY KEY IDENTITY,		\
				[gold] INT NULL								\
			);";

		DBConnection* dbConn = GDBConnectionPool->Pop();
		ASSERT_CRASH(dbConn->Execute(query)); 
		GDBConnectionPool->Push(dbConn);
	}

	// Add Data
	{
		for (int32 i = 0; i < 3; ++i) {
			DBConnection* dbConn = GDBConnectionPool->Pop();
			// 기존에 바인딩된 정보를 날림
			dbConn->Unbind();

			// 넘길 인자 바인딩
			int32 gold = 100 * i;
			SQLLEN len = 0;

			// 내가 어떤 형식의 데이터를 어떤 형식으로 넘겨줄지는 서술한다.
			ASSERT_CRASH(dbConn->BindParam(1, SQL_C_LONG, SQL_INTEGER, sizeof(gold), &gold, &len));

			// SQL 실행
			ASSERT_CRASH(dbConn->Execute(L"INSERT INTO [dbo].[Gold]([gold]) VALUE(?)"));
			GDBConnectionPool->Push(dbConn);
		}

	}

	// Read
	{
		DBConnection* dbConn = GDBConnectionPool->Pop();
		// 기존에 바인딩된 정보를 날림
		dbConn->Unbind();



		// 현재 BindParam이란건 (?)라고 되어잇는 녀석이 무엇인지 알려주는 역할을 한다.
		// 넘길 인자 바인딩
		int32 gold = 0;
		SQLLEN len = 0;

		// 내가 어떤 형식의 데이터를 어떤 형식으로 넘겨줄지는 서술한다.
		ASSERT_CRASH(dbConn->BindParam(1, SQL_C_LONG, SQL_INTEGER, sizeof(gold), &gold, &len));


		int32 outId = 0;
		SQLLEN outIdLen = 0;
		int32 outGold = 0;
		SQLLEN outGoldLen = 0;

		ASSERT_CRASH(dbConn->BindCol(1, SQL_C_LONG, sizeof(outId), &outId, &outIdLen));
		ASSERT_CRASH(dbConn->BindCol(2, SQL_C_LONG, sizeof(outGold), &outGold, &outGoldLen));

		ASSERT_CRASH(dbConn->Execute(L"SELECT id, gold FROM [dbo].[Gold] WHERE gold = (?)"));

		// Execute를 하면 바로 데이터가 넘겨오지않고 여러가지 데이터로 받아올수도 있다
		while (dbConn->Fetch()) {
			cout << "Id : " << outId << "Gold : " << outGold << endl;
		}
		GDBConnectionPool->Push(dbConn);
	}

	// -------------------------------------------------

	GRoom->DoTimer(1000, [] { cout << "Hello 1000" << endl; });
	GRoom->DoTimer(2000, [] { cout << "Hello 2000" << endl; });
	GRoom->DoTimer(3000, [] { cout << "Hello 3000" << endl; });

	ClientPacketHandler::Init();

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>, // TODO : SessionManager 등
		100);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 5; i++)
	{
		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}

	// Main Thread
	DoWorkerJob(service);

	GThreadManager->Join();
}