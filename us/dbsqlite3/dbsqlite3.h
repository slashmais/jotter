#ifndef _dbsqlite3_h_
#define _dbsqlite3_h_

#include <string>
#include <vector>
#include <sqlite3.h>
#include "dbsqlite3types.h"
#include <utilfuncs/utilfuncs.h>
#include <functional>

//--------------------------------------------------------------------------------------------------
struct DBsqlite3
{
	struct DBResult
	{
		struct ColVal
		{
			ColVal(const std::string sc,const std::string sv):col(sc),val(sv) {}
			std::string col, val;
		};
	
		typedef std::vector<ColVal> row_t;
	
		std::vector<row_t> Rows;
		std::string sQuery;
		void AddColVal(const std::string colname, const std::string value, row_t &r);
		void AddRow(row_t &r);
	
		DBResult();
		~DBResult();
		void Clear();
		const std::string GetQuery();
		int GetColCount();
		int GetRowCount();
		const std::string GetName(int nCol, int nRow);
		const std::string GetVal(int nCol, int nRow);
		const std::string GetVal(const std::string sCol, int nRow=0);
	//friend class DBsqlite3;
	};
	
	sqlite3 *pDB;
	std::string sDB;
	std::string sLastError;
	std::string sLastSQL;
	int RCLastError; //for sLastError
	DBResult *pRS;
	
	static int Callback(void *pCaller, int argc, char **argv, char **azColName);

	bool bDBOK;

	DBsqlite3();
	virtual ~DBsqlite3();

	std::function<void(bool)> WhenDBBusy;

	const char IDS_DELIM=(const char)',';

	bool init_ids(const std::string &id_name, bool bReset=false);
	DB_ID_TYPE new_id(const std::string &id_name);
	bool del_id(const std::string &id_name, DB_ID_TYPE id);
	bool isvalid_id(const std::string &id_name, DB_ID_TYPE id);
	
	bool Open(const std::string sDBName);
	void Close();
	size_t ExecSQL(DBResult *pRSet, const std::string sSQL);
	bool MakeTable(const std::string sT, const std::string sF, const std::string sC="");
	void SetLastError(const std::string &serr) { sLastError=serr; }
	const std::string GetLastError();
	int GetLastErrorRC();
	
	std::string GetDBName() { return path_name(sDB); }
	std::string GetDBPath() { return path_path(sDB); }
	std::string GetDBFullName() { return sDB; }
};


//--------------------------------------------------------------------------------------------------
enum /*SafeSql*/ { SS_NORMAL=0, SS_STARTLIKE, SS_CONTAINLIKE, SS_ENDLIKE, };
const std::string SQLSafeQ(const std::string &sval, int nlike=SS_NORMAL);
//const std::string SQLSafeQ(const std::string &sval); //..Q=>returned string wrapped in single-quotes
//const std::string SQLSafeQ_StartsLike(const std::string &sval); //("abc") -> 'abc%'
//const std::string SQLSafeQ_ContainsLike(const std::string &sval); //("abc") -> '%abc%'
//const std::string SQLSafeQ_EndsLike(const std::string &sval); //("abc") -> '%abc'

const std::string SQLRestore(const std::string &sval); //..of a SQLSafeQ()-string
bool NoError(DBsqlite3 *pdb);

template<typename P=int> std::string to(P p) { return says(p); } //specialized in .cpp
template<typename...T> void a_v_l(std::stringstream &s) {}
template<typename H, typename...T> void a_v_l(std::stringstream &ss, H h, T...t) { ss << to(h) << ","; a_v_l(ss, t...); }
template<typename...T> std::string SQLValues(T...t) //CAVEAT: not fully tested!
{
	std::string s;
	std::stringstream ss("");
	a_v_l(ss, t...);
	s=ss.str();
	TRIM(s, ",");
	s=says("VALUES(", s, ")");
	return s;
}

bool tellifdberror(bool b, DBsqlite3 *pdb);


#endif
