
#include "jdb.h"
#include "jconfig.h"
#include "jots.h"

#include <string>


//--------------------------------------------------------------------------------------------------
DB_Jotter DB;

//bool init_drop_words();

//--------------------------------------------------------------------------------------------------(schema)
bool DB_Jotter::ImplementSchema()
{
	DBResult RS;
	std::string sSQL;
	bool b=bDBOK;

	if (b)
	{
		sSQL = "CREATE TABLE IF NOT EXISTS IDS (name, id, del)";
		ExecSQL(&RS, sSQL); //reusable ids: init_ids(tabelname) once after CREATE TABLE..then new_id() / del_id() after
		b=NoError(this);
	}
	if (b) b = MakeTable("config", "x, y, w, h, scol");
	if (b) b = (MakeTable("jots", "idjot, jot, jtype, timestamp")&&init_ids("jots"));
	if (b) b = MakeTable("jtypes", "jtype");
	if (b) b = MakeTable("history", "jid, jot"); //deleted jots backup because!
	if (b) b = MakeTable("dropwords", "word"); //not used for indexing
	if (b) b = MakeTable("wordjotids", "word, jotids"); //jotids is csv of idjots (can be empty) (in jwords.h/cpp)
	if (b) b = db_init_jtypes();
	if (b) b = db_init_drop_words();

	return b;
}

//--------------------------------------------------------------------------------------------------(jot-types)
bool DB_Jotter::db_init_jtypes()
{
	bool b{true};
	if (b) b=SaveJType("jot");
	if (b) b=SaveJType("task");
	if (b) b=SaveJType("data");
	if (b) b=SaveJType("note");
	return b;
}

bool DB_Jotter::LoadJTypes(VUSTR &V)
{
	DBResult RS;
	size_t i=0,n;
	V.clear();
	n=ExecSQL(&RS, "SELECT * FROM jtypes");
	if (NoError(this))
	{
		std::string s{};
		while (i<n)
		{
			s=lcase(SQLRestore(RS.GetVal("jtype", i)));
			V.Add(s);
			i++;
		}
		return true;
	}
	return false;
}

bool DB_Jotter::SaveJType(const std::string &st)
{
	std::string sSQL;
	DBResult RS;
	sSQL=says("INSERT INTO jtypes(jtype) VALUES(", SQLSafeQ(lcase(st)), ")");
	ExecSQL(&RS, sSQL);
	return NoError(this);
}

bool DB_Jotter::DeleteJType(const std::string &st)
{
	if (sieqs(st, "jot")) return false;
	std::string sSQL;
	DBResult RS;
	sSQL=says("DELETE FROM jtypes WHERE jtype = ", SQLSafeQ(lcase(st)));
	ExecSQL(&RS, sSQL);
	return NoError(this);
}


//--------------------------------------------------------------------------------------------------(dropwords/wordjotids)
bool DB_Jotter::db_init_drop_words()
{
	std::string sSQL;
	DBResult RS;
	
	sSQL="INSERT INTO dropwords (word) VALUES"
		"('1'), ('2'), ('3'), ('4'), ('5'), ('6'), ('7'), ('8'), ('9')"
		", ('~'), ('!'), ('@'), ('#'), ('$'), ('%'), ('^'), ('&'), (','), ('.')"
		", ('*'), ('('), (')'), ('+'), ('='), ('{'), ('}'), ('[')"
		", (']'), ('|'), ('<'), ('>'), ('?'), ('/'), (';'), (':')"
		", ('a'), ('ain'), ('aint'), ('also'), ('am'), ('an'), ('and'), ('are'), ('as'), ('at')"
		", ('b'), ('be'), ('been'), ('but'), ('by')"
		", ('c'), ('can'), ('could')"
		", ('d'), ('did'), ('do'), ('does')"
		", ('e'), ('eg'), ('etc'), ('else')"
		", ('f'), ('for'), ('from')"
		", ('g'), ('go'), ('goes')"
		", ('h'), ('had'), ('has'), ('have'), ('he'), ('her'), ('here'), ('hers'), ('him'), ('his'), ('how')"
		", ('i'), ('ie'), ('if'), ('in'), ('is'), ('isn'), ('it'), ('its'), ('iv')"
		", ('j'), ('just')"
		", ('k')"
		", ('l'), ('let'), ('like')"
		", ('m'), ('me'), ('my')"
		", ('n'), ('not')"
		", ('o'), ('of'), ('on'), ('or'), ('our')"
		", ('p')"
		", ('q')"
		", ('r')"
		", ('s'), ('shall'), ('she'), ('should'), ('so')"
		", ('t'), ('than'), ('thanks'), ('that'), ('the'), ('their'), ('them'), ('then')"
		", ('there'), ('these'), ('they'), ('this'), ('those'), ('to'), ('too')"
		", ('u'), ('us')"
		", ('v'), ('ve'), ('very')"
		", ('w'), ('want'), ('was'), ('wasn'), ('we'), ('were'), ('what'), ('when'), ('where')"
		", ('which'), ('while'), ('who'), ('whom'), ('whose'), ('why'), ('with'), ('would')"
		", ('x')"
		", ('y'), ('yes'), ('yet'), ('you'), ('your'), ('yours')"
		", ('z');";
	ExecSQL(&RS, sSQL);
	return NoError(this);
}

//--------------------------------------------------------------------------------------------------(config)
bool DB_Jotter::Save(JotterConfig &con)
{
	std::string sSQL;
	DBResult RS;
	ExecSQL(&RS, "DELETE FROM config");
	if (NoError(this))
	{
		sSQL=says("INSERT INTO config(x, y, w, h, scol) VALUES(", con.x, ", ", con.y, ", ", con.w, ", ", con.h, ", ", con.scol, ")");
		ExecSQL(&RS, sSQL);
	}
	return NoError(this);
}

bool DB_Jotter::Load(JotterConfig &con)
{
	DBResult RS;
	size_t i=0,n;

	n = ExecSQL(&RS, "SELECT * FROM config");
	if (NoError(this))
	{
		if (n>0)
		{
			con.x = stot<int>(RS.GetVal("x", 0));
			con.y = stot<int>(RS.GetVal("y", 0));
			con.w = stot<int>(RS.GetVal("w", 0));
			con.h = stot<int>(RS.GetVal("h", 0));
			con.scol = stot<int>(RS.GetVal("scol", 0));
		}
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------(jot)
bool DB_Jotter::Save(Jot &J)
{
	std::string sSQL;
	DBResult RS;
	
	if (J.idJot)
	{
		sSQL=says("UPDATE jots SET",
					" jot = ", SQLSafeQ(J.sJot),
					", jtype = ", SQLSafeQ(J.jtype),
					" WHERE idjot = ", J.idJot);
		ExecSQL(&RS, sSQL);
	}
	else
	{
		J.idJot=new_id("jots");
		sSQL=says("INSERT INTO jots(idjot, jot, jtype, timestamp) VALUES(",
					J.idJot,
					", ", SQLSafeQ(J.sJot),
					", ", SQLSafeQ(J.jtype),
					", ", J.dtcreated,
					")");
		ExecSQL(&RS, sSQL);
	}
	return NoError(this);
}

bool DB_Jotter::Load(Jots &LJ)
{
	DBResult RS;
	size_t i=0,n;

	LJ.clear();
	n = ExecSQL(&RS, "SELECT * FROM jots");
	
	if (!NoError(this)) sayerr(GetLastError(), " \n", RS.GetQuery(), "\n");
	
	if (NoError(this))
	{
		while (i<n)
		{
			Jot J;
			J.idJot = stot<size_t>(RS.GetVal("idjot", i));
			J.sJot = SQLRestore(RS.GetVal("jot", i));
			J.jtype = SQLRestore(RS.GetVal("jtype", i));
			J.dtcreated = stot<DTStamp>(RS.GetVal("timestamp", i));
			LJ.add(J);
			i++;
		}
		return true;
	}
	return false;
}

bool DB_Jotter::DeleteJot(size_t id)
{
	std::string sSQL;
	DBResult RS;
	sSQL=says("DELETE FROM jots WHERE idjot = ", id);
	ExecSQL(&RS, sSQL);
	if (NoError(this)) return del_id("jots", id);
	return false;
}

//--------------------------------------------------------------------------------------------------(history)
bool DB_Jotter::SaveHistory(Jot &J)
{
	std::string sSQL;
	DBResult RS;
	if (J.idJot)
	{
		DeleteHistory(J.idJot); //just in case
		sSQL=says("INSERT INTO history(jid, jot) VALUES(", J.idJot, ", ", SQLSafeQ(J.sJot), ")");
		ExecSQL(&RS, sSQL);
	}
	return NoError(this);
}

bool DB_Jotter::LoadHistoryJot(size_t id, Jot &N)
{
	std::string sSQL;
	DBResult RS;
	N.clear();
	sSQL=says("SELECT * FROM history WHERE jid = ", id);
	ExecSQL(&RS, sSQL);
	if (NoError(this))
	{
		N.sJot = SQLRestore(RS.GetVal("jot", 0));
		return true;
	}
	return false;
}

bool DB_Jotter::LoadHistory(Jots &L)
{
	DBResult RS;
	size_t i=0,n;

	L.clear();
	n = ExecSQL(&RS, "SELECT * FROM history;");
	if (NoError(this))
	{
		while (i<n)
		{
			Jot J;
			J.idJot = stot<size_t>(RS.GetVal("jid", i));
			J.sJot = SQLRestore(RS.GetVal("jot", i));
			L.add(J);
			i++;
		}
		return true;
	}
	return false;
}

bool DB_Jotter::DeleteHistory(size_t id)
{
	std::string sSQL;
	DBResult RS;
	sSQL=says("DELETE FROM history WHERE jid = ", id);
	ExecSQL(&RS, sSQL);
	return NoError(this);
}

