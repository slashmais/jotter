#ifndef _jotter_h_
#define _jotter_h_

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <jotter/jotter.lay>
#include <CtrlCore/lay.h>

#include "jconfig.h"
#include <utilfuncs/utilfuncs.h>
#include "jots.h"
#include "jdb.h"
#include "jwords.h"


struct Jotter : public WithjotterLayout<TopWindow>
{
	Jots jots;
	VUSTR vjtypes;
	JotterConfig con;
	size_t curid{0};
	JotWords JWords;
	
	void get_xywh(Ctrl *pC, int &X, int &Y, int &W, int &H);
	void save_config();
	bool load_config();
	
	~Jotter();
	Jotter();
	
	void Close();

	bool get_search_ids(VID &sids);
	void FillArt();
	void ARMenu(Bar &bar);
	
	bool add_jot(Jot &jot);

	bool add_jtype(const std::string &sn); //true if not exist
	void del_jtype(const std::string &sn);

	void EditJot(size_t id=0);
	void DeleteJot(size_t id);
	void ShowJot(size_t id=0);
	
	void DoDropwords();

	void ShowHistory();
	
	bool Key(dword key, int count);
	
};

struct UIJot : public WithuijotLayout<TopWindow>
{
	Jotter *pJ;
	bool bOK;
	~UIJot() {}
	UIJot(Jotter *pj=nullptr);
	bool Key(dword key, int count);
};

struct UIDrops : public WithdropwordLayout<TopWindow>
{
	JotWords *pJW{nullptr};
	~UIDrops() {}
	UIDrops(JotWords *p);
	void fill_cldrops();
	void DoAddDW();
	void DoDelDW(const std::string &w);
	bool Key(dword key, int count);
};

struct UIHistory : public WithhistoryLayout<TopWindow>
{
	Jotter *pJ{nullptr};
	~UIHistory();
	UIHistory(Jotter *pj=nullptr);
	void fillarh();
	void arhmenu(Bar &bar);
	bool Restore(size_t id);
	void Destroy(size_t id, bool bask=true);
};

#endif
