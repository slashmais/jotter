#ifndef _jwords_h_
#define _jwords_h_

#include <dbsqlite3/dbsqlite3.h>
#include <string>
#include <utilfuncs/utilfuncs.h>
#include <vector>
#include <map>


typedef VType<size_t> VID;


struct JotWords
{
	DBsqlite3 *pDB{nullptr};
	std::map<std::string, int> mdrop{}; //dropwords in mem-buffer
	
	~JotWords();
	JotWords(DBsqlite3 *pdb);

	bool fill_mdrop();
	
	bool IsDropword(const std::string &w);
	bool AddDropword(const std::string &w);
	bool RemoveDropword(const std::string &w);
	
	bool AddWordJID(const std::string &w, DB_ID_TYPE jid);
	bool DeleteWord(const std::string &w);
	bool DeleteJID(DB_ID_TYPE id);
	bool GetWordJIDList(const std::string &w, VID *pv); //clears & sets pv
	bool GetRedactJIDList(const std::string &w, VID *pv); //changes pv based on list for w
	void AddData(const std::string sdata, DB_ID_TYPE jid); //extract (utf8)words & update list
	
};





#endif
