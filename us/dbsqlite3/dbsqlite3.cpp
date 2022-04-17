
#include "dbsqlite3.h"
#include <utilfuncs/utilfuncs.h>
#include <string>


//--------------------------------------------------------------------------------------------------
template<> std::string to(std::string s)		{ return SQLSafeQ(s); }
template<> std::string to(std::string &s)		{ return SQLSafeQ(s); }
template<> std::string to(const std::string &s)	{ return SQLSafeQ(s); }
template<> std::string to(const char *s)		{ return SQLSafeQ(s); }
template<> std::string to(char *s)				{ return SQLSafeQ(s); }
//did I miss any?

//--------------------------------------------------------------------------------------------------
DBsqlite3::DBResult::DBResult()					{ Clear(); }
DBsqlite3::DBResult::~DBResult()				{ Clear(); }

void DBsqlite3::DBResult::Clear()				{ Rows.clear(); sQuery.clear(); }
void DBsqlite3::DBResult::AddColVal(const std::string scol, const std::string sval, row_t &r) { r.push_back(ColVal(scol,sval)); }
void DBsqlite3::DBResult::AddRow(row_t &r)		{ Rows.push_back(r); }

const std::string DBsqlite3::DBResult::GetQuery()	{ return sQuery; }

int DBsqlite3::DBResult::GetColCount()			{ return ((Rows.size()>0)?Rows.at(0).size():0); }
int DBsqlite3::DBResult::GetRowCount()			{ return Rows.size(); }

const std::string DBsqlite3::DBResult::GetName(int nCol, int nRow)
{
	return (((nRow<(int)Rows.size())&&(nCol<GetColCount()))?Rows.at(nRow).at(nCol).col:"");
}

const std::string DBsqlite3::DBResult::GetVal(int nCol, int nRow)
{
	return (((nRow<(int)Rows.size())&&(nCol<GetColCount()))?Rows.at(nRow).at(nCol).val:"");
}

const std::string DBsqlite3::DBResult::GetVal(const std::string sCol, int nRow)
{
	if (nRow<(int)Rows.size())
	{
		auto it = Rows.at(nRow).begin();
		while (it != Rows.at(nRow).end()) { if (sieqs((*it).col, sCol)) return ((*it).val); it++; }
	}
	return "";
}

//--------------------------------------------------------------------------------------------------
DBsqlite3::DBsqlite3()
{
	bDBOK=false;
	pRS=NULL;
	sDB.clear();
	sLastError.clear();
	sLastSQL.clear();
	RCLastError=SQLITE_OK;
	WhenDBBusy=nullptr;
}

DBsqlite3::~DBsqlite3() { Close(); }

bool DBsqlite3::init_ids(const std::string &tname, bool bReset)
{
	DBResult RS;
	std::string SQL;

	SQL=says("SELECT * FROM IDS WHERE name = ", SQLSafeQ(tname));
	if (ExecSQL(&RS, SQL)>0)
	{
		if (!bReset) return true; //exists
		else
		{
			SQL=says("DELETE FROM IDS WHERE name = ", SQLSafeQ(tname));
			ExecSQL(&RS, SQL);
		}
	}
	SQL=says("INSERT INTO IDS (name, id, del) VALUES (", SQLSafeQ(tname), ", 0, '')");
	ExecSQL(&RS, SQL);
	return NoError(this);
}

DB_ID_TYPE DBsqlite3::new_id(const std::string &tname)
{
	std::string sSQL, s;
	DBResult RS;
	DB_ID_TYPE id=0;
	
	sSQL=says("SELECT id, del FROM IDS WHERE name = ", SQLSafeQ(tname));
	ExecSQL(&RS, sSQL);
	if (NoError(this))
	{
		VType<DB_ID_TYPE> v;
		s=SQLRestore(RS.GetVal("del", 0));
		if (desv<DB_ID_TYPE>(s, IDS_DELIM, v, false)>0)
		{
			id=v[0];
			v.erase(v.begin());
			ensv<DB_ID_TYPE>(v, IDS_DELIM, s, false);
			sSQL=says("UPDATE IDS SET del = ", SQLSafeQ(s), " WHERE name = ", SQLSafeQ(tname));
			ExecSQL(&RS, sSQL);
			if (!NoError(this)) id=0;
		}
		else
		{
			id=stot<DB_ID_TYPE>(RS.GetVal("id",0))+1;
			sSQL=says("UPDATE IDS SET id = ", id, " WHERE name = ", SQLSafeQ(tname));
			ExecSQL(&RS, sSQL);
			if (!NoError(this)) id=0;
		}
	}
	else say("ERROR: ", sLastError);
	return id;
}

bool DBsqlite3::del_id(const std::string &tname, DB_ID_TYPE id)
{
	std::string sSQL, s;
	DBResult RS;
	
	sSQL=says("SELECT id, del FROM IDS WHERE name = ", SQLSafeQ(tname));
	ExecSQL(&RS, sSQL);
	if (NoError(this))
	{
		VType<DB_ID_TYPE> v;
		s=SQLRestore(RS.GetVal("del", 0));
		desv<DB_ID_TYPE>(s, IDS_DELIM, v, false);
		v.push_back(id);
		ensv<DB_ID_TYPE>(v, IDS_DELIM, s, false);
		sSQL=says("UPDATE IDS SET del = ", SQLSafeQ(s), " WHERE name = ", SQLSafeQ(tname));
		ExecSQL(&RS, sSQL);
	}
	return NoError(this);
}

bool DBsqlite3::isvalid_id(const std::string &id_name, DB_ID_TYPE id)
{
	if (!id) return false;
	//must be less than or equal to IDS.id, and not in IDS.del
	std::string sSQL, s;
	DBResult RS;
	
	sSQL=says("SELECT id, del FROM IDS WHERE name = ", SQLSafeQ(id_name));
	ExecSQL(&RS, sSQL);
	if (NoError(this))
	{
		if (id>stot<DB_ID_TYPE>(RS.GetVal("id",0))) return false;
		VType<DB_ID_TYPE> v;
		s=SQLRestore(RS.GetVal("del", 0));
		desv<DB_ID_TYPE>(s, IDS_DELIM, v, false);
		for (auto i:v) { if (i==id) return false; }
		return true;
	}
	return false;
}

/*static*/
int DBsqlite3::Callback(void *pCaller, int argc, char **argv, char **azColName)
{
	DBsqlite3 *pDB=(DBsqlite3*)pCaller;
	if (!pDB) return 1;
	if (argc>0)
	{
		DBResult::row_t r;
		for(int i=0; i<argc; i++) pDB->pRS->AddColVal(azColName[i], ((argv[i])?argv[i]:"NULL"), r);
		pDB->pRS->AddRow(r);
	}
	return 0;
}

bool DBsqlite3::Open(const std::string sDBName)
{
	sLastError.clear();
	sLastSQL.clear();
	if (!bDBOK)
	{
		sDB = sDBName;
		bDBOK = (sqlite3_open(sDB.c_str(), &pDB)==0);
		if (!bDBOK) sLastError = "Cannot open database";
	}
	return bDBOK;
}

void DBsqlite3::Close() { if (bDBOK) sqlite3_close(pDB); bDBOK=false; }
	
size_t DBsqlite3::ExecSQL(DBResult *pRSet, const std::string sSQL)
{
	if (!bDBOK) { sLastError = "Database not open"; return 0; }
	if (WhenDBBusy) WhenDBBusy(true); //if a UI wants to show something..
	//sLastSQL=sSQL;
	char *szErrMsg = 0;
	pRSet->Clear();
	pRSet->sQuery = sSQL;
	//sLastError.clear();
	pRS = pRSet;
	int rc = sqlite3_exec(pDB, sSQL.c_str(), &DBsqlite3::Callback, this, &szErrMsg);
	if (rc!=SQLITE_OK) { RCLastError=rc; sLastError=szErrMsg; sqlite3_free(szErrMsg); }
	sLastSQL=sSQL;
	if (WhenDBBusy) WhenDBBusy(false);
	return pRSet->GetRowCount();
}

bool DBsqlite3::MakeTable(const std::string sT, const std::string sF, const std::string sC)
{
	DBResult RS;
	std::string sSQL;
	sSQL=says("CREATE TABLE IF NOT EXISTS ", sT.c_str(), " (", sF.c_str(), ") ", sC);
	ExecSQL(&RS, sSQL);
	return (sLastError.empty());
}

const std::string DBsqlite3::GetLastError()		{ return sLastError; }

int DBsqlite3::GetLastErrorRC()					{ return RCLastError; }

//--------------------------------------------------------------------------------------------------
//todo ??? '_' '%' ';' ??? -  necessary? all are strings in sqlite, so, not necessary
void make_sql_safe(std::string &s) { ReplaceChars(s, "\"", "\"\""); ReplaceChars(s, "'", "''"); }

const std::string SQLSafeQ(const std::string &sval, int nlike)
{
	std::string t=sval, r;
	make_sql_safe(t);
	switch(nlike)
	{
		case SS_STARTLIKE:		r=says("'", t, "%'");
		case SS_CONTAINLIKE:	r=says("'%", t, "%'");
		case SS_ENDLIKE:		r=says("'%", t, "'");
		default:				r=says("'", t, "'");
	}
	return r;
}

const std::string SQLRestore(const std::string &sval)
{
	std::string r=sval;
	if (!r.empty())
	{
		ReplacePhrase(r, "\"\"", "\"");
		ReplacePhrase(r, "''", "'");
	}
	return r;
}

bool NoError(DBsqlite3 *pdb)
{
	return (pdb->GetLastError().empty());
}

//--------------------------------------------------------------------------------------------------
bool tellifdberror(bool b, DBsqlite3 *pdb)
{
	if (!b) sayerr(path_name(pdb->sDB), ": [", pdb->GetLastErrorRC(), "] ", pdb->GetLastError());
	return b;
}
