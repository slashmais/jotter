#ifndef _dbjotter_h_
#define _dbjotter_h_

#include <utilfuncs/utilfuncs.h>
#include <dbsqlite3/dbsqlite3.h>


//--------------------------------------------------------------------------------------------------
struct JotterConfig;
struct Jot;
struct Jots;

struct DB_Jotter : public DBsqlite3
{
	virtual ~DB_Jotter() {}
	
	bool ImplementSchema();

	bool db_init_jtypes();
	bool LoadJTypes(VUSSTR &V);
	bool SaveJType(const std::string &st);
	bool DeleteJType(const std::string &st);

	bool db_init_drop_words();
	
	bool Save(JotterConfig &con);
	bool Load(JotterConfig &con);

	bool Save(Jot &N);
	bool Load(Jots &L);
	bool DeleteJot(size_t id);

	bool SaveHistory(Jot &N);
	bool LoadHistoryJot(size_t id, Jot &N);
	bool LoadHistory(Jots &L);
	bool DeleteHistory(size_t id);


//	bool ValidateHasTable(const std::string &tname);

};

extern DB_Jotter DB;




#endif
