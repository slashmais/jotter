
#include "jwords.h"


//===================================================================================================(JotIDList)
struct JotIDList
{
	VID *pvid{nullptr};
	const char DELIM=(const char)',';
	
	void clear()									{ if (pvid) pvid->clear(); }

	~JotIDList()									{ }
	JotIDList(VID *pv)								{ pvid=pv; clear(); }

	size_t size() const								{ return (pvid)?pvid->size():0; }

	DB_ID_TYPE& operator[](size_t i)				{ if (pvid&&(i<size())) return (*pvid)[i]; throw std::out_of_range("JotIDList"); }
	const DB_ID_TYPE& operator[](size_t i) const	{ if (pvid&&(i<size())) return (*pvid)[i]; throw std::out_of_range("JotIDList"); }

	bool add_id(DB_ID_TYPE id);
	bool del_id(DB_ID_TYPE id);
	bool has_id(DB_ID_TYPE id);
	bool put_list(const std::string &csv);
	bool get_list(std::string &csv);
};

bool JotIDList::add_id(DB_ID_TYPE id)
{
	if (!pvid) return false;
	if (!has_id(id)) pvid->push_back(id);
	return true;
}

bool JotIDList::del_id(DB_ID_TYPE id)
{
	if (!pvid) return false;
	auto it=pvid->begin();
	while (it!=pvid->end())
	{
		if ((*it)==id) { pvid->erase(it); break; }
		it++;
	}
	return true;
}

bool JotIDList::has_id(DB_ID_TYPE id)
{
	if (!pvid) return false;
	for (auto i:(*pvid)) { if (i==id) return true; }
	return false;
}
	
bool JotIDList::put_list(const std::string &csv)
{
	if (!pvid) return false;
	pvid->clear();
	desv<DB_ID_TYPE>(csv, DELIM, *pvid, false);
	return true;
}

bool JotIDList::get_list(std::string &csv)
{
	if (!pvid) return false;
	csv.clear();
	for (auto i:(*pvid)) { if (!csv.empty()) csv+=DELIM; csv+=ttos<size_t>(i); }
	return (csv.size()>0);
}


//===================================================================================================(JotWords)
JotWords::~JotWords()
{
	mdrop.clear();
}

JotWords::JotWords(DBsqlite3 *pdb) : pDB{pdb}
{
	//pDB=pdb;
	fill_mdrop();
}

bool JotWords::fill_mdrop()
{
	mdrop.clear();
	std::string sSQL, s;
	DBsqlite3::DBResult RS;
	sSQL=says("SELECT word FROM dropwords ORDER BY word ASC");
	size_t n=pDB->ExecSQL(&RS, sSQL);
	for (size_t i=0; i<n; i++)
	{
		s=SQLRestore(RS.GetVal("word", i));
		mdrop[s]=0;
	}
	return NoError(pDB);
}

bool JotWords::IsDropword(const std::string &w)
{
	return (mdrop.find(w)!=mdrop.end());
}

bool JotWords::AddDropword(const std::string &w)
{
	if (IsDropword(w)) return true;
	std::string sSQL;
	DBsqlite3::DBResult RS;
	sSQL=says("INSERT INTO dropwords(word) VALUES(", SQLSafeQ(w), ")");
	pDB->ExecSQL(&RS, sSQL);
	if (NoError(pDB)) return fill_mdrop();
	return false;
}

bool JotWords::RemoveDropword(const std::string &w)
{
	std::string sSQL;
	DBsqlite3::DBResult RS;
	sSQL=says("DELETE FROM dropwords WHERE word=", SQLSafeQ(w));
	pDB->ExecSQL(&RS, sSQL);
	if (NoError(pDB)) return fill_mdrop();
	return false;
}


//--------------------------------------------------(wordjotids)
bool JotWords::AddWordJID(const std::string &w, DB_ID_TYPE jid)
{
	if (IsDropword(w)) return false;

	std::string sSQL, s;
	DBsqlite3::DBResult RS;
	VID v;
	JotIDList jidl(&v);
	if (GetWordJIDList(w, &v))
	{
		jidl.add_id(jid);
		jidl.get_list(s);
		sSQL=says("UPDATE wordjotids SET jotids='", s, "' WHERE word=", SQLSafeQ(w));
		pDB->ExecSQL(&RS, sSQL);
	}
	else
	{
		s=says(jid); //new, so just this id
		sSQL=says("INSERT INTO wordjotids(word, jotids) VALUES(", SQLSafeQ(w), ", '", s, "')");
		pDB->ExecSQL(&RS, sSQL);
	}
	return NoError(pDB);
}

bool JotWords::DeleteWord(const std::string &w)
{
	std::string sSQL;
	DBsqlite3::DBResult RS;
	sSQL=says("DELETE FROM wordjotids WHERE word=", SQLSafeQ(w));
	pDB->ExecSQL(&RS, sSQL);
	return NoError(pDB);
}

bool JotWords::DeleteJID(DB_ID_TYPE id)
{
	std::string sSQL, s;
	std::map<std::string, std::string> m{};
	DBsqlite3::DBResult RS;

	sSQL=says("SELECT * FROM wordjotids");
	size_t i=0, n=pDB->ExecSQL(&RS, sSQL);
	while (i<n)
	{
		m[SQLRestore(RS.GetVal("word", i))]=RS.GetVal("ndis", i);
		i++;
	}
	for (auto& p:m)
	{
		VID v;
		JotIDList jidl(&v);
		jidl.put_list(p.second);
		if (jidl.has_id(id))
		{
			jidl.del_id(id);
			jidl.get_list(s);
			sSQL=says("UPDATE wordjotids SET jotids='", s, "' WHERE word=", SQLSafeQ(p.first));
			pDB->ExecSQL(&RS, sSQL);
		}
		if (!NoError(pDB)) return false;
	}
	return true;
}

bool JotWords::GetWordJIDList(const std::string &wrd, VID *pv)
{
	JotIDList jidl(pv);
	std::string sSQL{"SELECT jotids FROM wordjotids WHERE word LIKE "};
	std::string s{wrd};
	DBsqlite3::DBResult RS;
	int ssflag=SS_CONTAINLIKE;
	sSQL+=SQLSafeQ(s, ssflag);
	size_t n=pDB->ExecSQL(&RS, sSQL);
	if (!NoError(pDB)) sayerr(pDB->GetLastError(), " \n", sSQL, "\n");
	if (n) { s=RS.GetVal("jotids", 0); jidl.put_list(s); /* fills pv */ }
	return (jidl.size()>0);
	
}
	
bool JotWords::GetRedactJIDList(const std::string &w, VID *pv)
{
	JotIDList jidl(pv);
	if (!jidl.size()) return false; //already empty (initial fill with GetWordJIDList() )
	VID v{};
	JotIDList l(&v);
	GetWordJIDList(w, &v);
	size_t i=0;
	while (i<jidl.size())
	{
		if (!l.has_id(jidl[i])) jidl.del_id(jidl[i]);
		else i++;
	}
	return (jidl.size()>0);
}

void JotWords::AddData(const std::string sdata, DB_ID_TYPE jid)
{
	const std::string D=std::string("1234567890=~!@#$%^&*()+\";:[]{}|\\<>?,./"); //todo: add Unicode-punctuation chars
	const std::string C=std::string("`-_'"); //possible connectors
	std::string w{};
	size_t i=0, n=sdata.size();
	auto isws=[](char c)->bool{ return ((c==' ')||(c=='\n')||(c=='\r')||(c=='\t')); };
	auto isv=[&D, isws](char c)->bool{ return (D.find(c)==std::string::npos); };
	auto isC=[&C](char c)->bool{ return (C.find(c)!=std::string::npos); };
    while (i<n)
	{
		if (isv(sdata[i])&&!isws(sdata[i])&&!isC(sdata[i])) { w+=sdata[i]; }
		else if (isC(sdata[i])&&((i>0)&&(i<(n-1))&&isv(sdata[i-1])&&isv(sdata[i+1]))) { w+=sdata[i]; }
		else { if (w.size()) { AddWordJID(w, jid); w.clear(); }}
		i++;
	}
	if (w.size()) AddWordJID(w, jid);
}

