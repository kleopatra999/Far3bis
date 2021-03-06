/*
sqlitedb.cpp

������ sqlite api ��� c++.
*/
/*
Copyright � 2011 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "sqlitedb.hpp"
#include "sqlite.hpp"
#include "pathmix.hpp"
#include "config.hpp"
#include "synchro.hpp"
#include "components.hpp"

static string getInfo() { return L"SQLite, version " + wide(SQLITE_VERSION); }
SCOPED_ACTION(components::component)(getInfo);

static void GetDatabasePath(const string& FileName, string &strOut, bool Local)
{
	if(FileName != L":memory:")
	{
		strOut = Local ? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath;
		AddEndSlash(strOut);
		strOut += FileName;
	}
	else
	{
		strOut = FileName;
	}
}

SQLiteDb::SQLiteStmt::SQLiteStmt():
	param(1),
	pStmt(nullptr)
{
}

SQLiteDb::SQLiteStmt::~SQLiteStmt()
{
	Finalize();
}

bool SQLiteDb::SQLiteStmt::Finalize()
{
	if (pStmt)
		return sqlite::sqlite3_finalize(pStmt) == SQLITE_OK;
	else
		return true;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::Reset()
{
	param=1;
	sqlite::sqlite3_clear_bindings(pStmt);
	sqlite::sqlite3_reset(pStmt);
	return *this;
}

bool SQLiteDb::SQLiteStmt::Step()
{
	return sqlite::sqlite3_step(pStmt) == SQLITE_ROW;
}

bool SQLiteDb::SQLiteStmt::StepAndReset()
{
	bool b = sqlite::sqlite3_step(pStmt) == SQLITE_DONE;
	Reset();
	return b;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::Bind(int Value)
{
	sqlite::sqlite3_bind_int(pStmt,param++,Value);
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::Bind(__int64 Value)
{
	sqlite::sqlite3_bind_int64(pStmt,param++,Value);
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::Bind(const string& Value, bool bStatic)
{
	using sqlite::sqlite3_destructor_type; // for SQLITE_* macros
	sqlite::sqlite3_bind_text16(pStmt,param++,Value.data(),-1,bStatic? SQLITE_STATIC : SQLITE_TRANSIENT);
	return *this;
}

SQLiteDb::SQLiteStmt& SQLiteDb::SQLiteStmt::Bind(const void *Value, size_t Size, bool bStatic)
{
	using sqlite::sqlite3_destructor_type; // for SQLITE_* macros
	sqlite::sqlite3_bind_blob(pStmt, param++, Value, static_cast<int>(Size), bStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
	return *this;
}

const wchar_t* SQLiteDb::SQLiteStmt::GetColText(int Col)
{
	return (const wchar_t *)sqlite::sqlite3_column_text16(pStmt,Col);
}

const char* SQLiteDb::SQLiteStmt::GetColTextUTF8(int Col)
{
	return (const char *)sqlite::sqlite3_column_text(pStmt,Col);
}

int SQLiteDb::SQLiteStmt::GetColBytes(int Col)
{
	return sqlite::sqlite3_column_bytes(pStmt,Col);
}

int SQLiteDb::SQLiteStmt::GetColInt(int Col)
{
	return sqlite::sqlite3_column_int(pStmt,Col);
}

unsigned __int64 SQLiteDb::SQLiteStmt::GetColInt64(int Col)
{
	return sqlite::sqlite3_column_int64(pStmt,Col);
}

const char* SQLiteDb::SQLiteStmt::GetColBlob(int Col)
{
	return (const char *)sqlite::sqlite3_column_blob(pStmt,Col);
}

SQLiteDb::ColumnType SQLiteDb::SQLiteStmt::GetColType(int Col)
{
	switch (sqlite::sqlite3_column_type(pStmt,Col))
	{
	case SQLITE_INTEGER:
		return TYPE_INTEGER;
	case SQLITE_TEXT:
		return TYPE_STRING;
	case SQLITE_BLOB:
		return TYPE_BLOB;
	default:
		return TYPE_UNKNOWN;
	}
}


SQLiteDb::SQLiteDb():
	pDb(nullptr), init_status(-1), db_exists(-1)
{
}

SQLiteDb::~SQLiteDb()
{
	Close();
}

static bool can_create_file(const string& fname)
{
	return api::File().Open(fname, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE);
}

bool SQLiteDb::Open(const string& DbFile, bool Local, bool WAL)
{
	GetDatabasePath(DbFile, strPath, Local);
	bool mem_db = DbFile == L":memory:";

	if (!Global->Opt->ReadOnlyConfig || mem_db)
	{
		if (!mem_db && db_exists < 0) {
			DWORD attrs = api::GetFileAttributes(strPath);
			db_exists = (0 == (attrs & FILE_ATTRIBUTE_DIRECTORY)) ? +1 : 0;
		}
		bool ret = (SQLITE_OK == sqlite::sqlite3_open16(strPath.data(), &pDb));
		if (ret)
			sqlite::sqlite3_busy_timeout(pDb, 1000);
		return ret;
	}

	// copy db to memory
	//
	if (SQLITE_OK != sqlite::sqlite3_open16(L":memory:", &pDb))
		return false;

	bool ok = true, copied = false;
	sqlite::sqlite3 *db_source = nullptr;

	DWORD attrs = api::GetFileAttributes(strPath);
	if (0 == (attrs & FILE_ATTRIBUTE_DIRECTORY)) // source exists and not directory
	{
		if (db_exists < 0)
			db_exists = +1;
		UUID Id;
		UuidCreate(&Id);
		if (WAL && !can_create_file(strPath + L"." + GuidToStr(Id))) // can't open db -- copy to %TEMP%
		{
			FormatString strTmp;
			api::GetTempPath(strTmp);
			strTmp << GetCurrentProcessId() << L'-' << DbFile;
			ok = copied = FALSE != api::CopyFileEx(strPath, strTmp, nullptr, nullptr, nullptr, 0);
			api::SetFileAttributes(strTmp, attrs & ~(FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM));
			if (ok)
				strPath = strTmp;
			ok = ok && (SQLITE_OK == sqlite::sqlite3_open16(strPath.data(), &db_source));
		}
		else
		{
			Utf8String name8(strPath);
			int flags = (WAL ? SQLITE_OPEN_READWRITE : SQLITE_OPEN_READONLY);
			ok = (SQLITE_OK == sqlite::sqlite3_open_v2(name8.data(), &db_source, flags, nullptr));
		}
		if (ok)
		{
			sqlite::sqlite3_busy_timeout(db_source, 1000);
			sqlite::sqlite3_backup *db_backup = sqlite::sqlite3_backup_init(pDb, "main", db_source, "main");
			ok = (nullptr != db_backup);
			if (ok)
			{
				sqlite::sqlite3_backup_step(db_backup, -1);
				sqlite::sqlite3_backup_finish(db_backup);
				int rc = sqlite::sqlite3_errcode(pDb);
				ok = (SQLITE_OK == rc);
			}
		}
	}

	if (db_source)
		sqlite::sqlite3_close(db_source);
	if (copied)
		api::DeleteFile(strPath);

	strPath = L":memory:";
	if (!ok)
		Close();
	return ok;
}

void SQLiteDb::Initialize(const string& DbName, bool Local)
{
	string &path = Local ? Global->Opt->LocalProfilePath : Global->Opt->ProfilePath;

	Mutex m(path.data(), DbName.data());
	SCOPED_ACTION(lock_guard<Mutex>)(m);

	m_Name = DbName;
	init_status = 0;

	if (!InitializeImpl(DbName, Local))
	{
		Close();
		++init_status;

		bool in_memory = (Global->Opt->ReadOnlyConfig != 0) ||
			!api::MoveFileEx(strPath, strPath+L".bad",MOVEFILE_REPLACE_EXISTING) || !InitializeImpl(DbName,Local);

		if (in_memory)
		{
			Close();
			++init_status;
			InitializeImpl(L":memory:", Local);
		}
	}
}

int SQLiteDb::InitStatus(string& name, bool full_name)
{
	name = (full_name && !strPath.empty() && strPath != L":memory:") ? strPath : m_Name;
	return init_status;
}

bool SQLiteDb::Exec(const char *Command)
{
	return sqlite::sqlite3_exec(pDb, Command, nullptr, nullptr, nullptr) == SQLITE_OK;
}

bool SQLiteDb::BeginTransaction()
{
	return Exec("BEGIN TRANSACTION;");
}

bool SQLiteDb::EndTransaction()
{
	return Exec("END TRANSACTION;");
}

bool SQLiteDb::RollbackTransaction()
{
	return Exec("ROLLBACK TRANSACTION;");
}

bool SQLiteDb::InitStmt(SQLiteStmt &stmtStmt, const wchar_t *Stmt)
{
	return sqlite::sqlite3_prepare16_v2(pDb, Stmt, -1, &stmtStmt.pStmt, nullptr) == SQLITE_OK;
}

int SQLiteDb::Changes()
{
	return sqlite::sqlite3_changes(pDb);
}

unsigned __int64 SQLiteDb::LastInsertRowID()
{
	return sqlite::sqlite3_last_insert_rowid(pDb);
}

bool SQLiteDb::Close()
{
	bool Result = sqlite::sqlite3_close(pDb) == SQLITE_OK;
	pDb = nullptr;
	return Result;
}

bool SQLiteDb::SetWALJournalingMode()
{
	return Exec("PRAGMA journal_mode = WAL;");
}

bool SQLiteDb::EnableForeignKeysConstraints()
{
	return Exec("PRAGMA foreign_keys = ON;");
}
