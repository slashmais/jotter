#include "jotter.h"
//#include "jdb.h"
#include "pictures.h"

int WFRAMEWIDTH;
int WTITLEBARHEIGHT;

//===================================================================================================
void Jotter::get_xywh(Ctrl *pC, int &X, int &Y, int &W, int &H)
{
	//adjust for windowframe
	Rect r=pC->GetRect();
	X=r.left-WFRAMEWIDTH;
	Y=r.top-WTITLEBARHEIGHT;
	W=r.Width();
	H=r.Height();
}

void Jotter::save_config()
{
	if (IsMinimized()) Restore();
	get_xywh(this, con.x, con.y, con.w, con.h);
	DB.Save(con);
}

bool Jotter::load_config() { return DB.Load(con); }

Jotter::~Jotter() { save_config(); }

Jotter::Jotter() : JWords(&DB)
{
	Icon(GetPic(JOTTERICON));
	CtrlLayout(*this, "Jotter");
	if (load_config()) { SetRect(con.x, con.y, con.w, con.h); } else CenterScreen();
	Sizeable();
	vjtypes.setunique();
	vjtypes.setsortorder(SORT_ASC);

	btnDropwords.WhenAction = [this](){ DoDropwords(); };
	btnHistory.WhenAction = [this]{ ShowHistory();};
	btnClose.WhenAction = [this]{Minimize();};

	arJots.WhenBar = [this](Bar &bar){ ARMenu(bar); };
	arJots.WhenLeftDouble = [this]{ ShowJot(); };
	arJots.WhenColumnSorted = [this]{ con.scol=arJots.GetSortColumn(); }; //save_config(); };

	ebSearch.WhenAction = [this](){ FillArt(); };
	
	DB.LoadJTypes(vjtypes);
	DB.Load(jots);
	
	FillArt();
}

void Jotter::Close()
{
	if (PromptOKCancel(DeQtf("Are you sure you want to Exit Jotter?"))) { save_config(); TopWindow::Close(); }
}

bool Jotter::get_search_ids(VID &sids)
{
	std::string s{};
	s=ebSearch.GetData().ToStd();
	TRIM(s);
	sids.clear();
	if (!s.empty())
	{
		VSTR v{};
		if (desv(s, ' ', v, false, true))
		{
			JWords.GetWordJIDList(v[0], &sids);
			for (size_t i=1; ((i<v.size())&&!sids.empty()); i++) { JWords.GetRedactJIDList(v[i], &sids); }
		}
	}
	return (!s.empty());
}

void Jotter::FillArt()
{
	arJots.Clear();
	arJots.Reset();
	arJots.AddColumn("", 0); //need to be hidden
	arJots.AddColumn("Created", 50).Sorting();
	arJots.AddColumn("Entry", 200).Sorting();
	arJots.AddColumn("Type", 40).Sorting();
	
	VID sids{};
	bool bSearching{false};
	bSearching=get_search_ids(sids);
	auto fid=[&](DB_ID_TYPE id)->bool{ for (auto i:sids) if (i==id) return true; return false; };
	
	for (auto N:jots)
	{
		if (bSearching&&!fid(N.idJot)) continue;
		std::string s{};
		s=N.sJot;
		arJots.Add(int(N.idJot), ToDateStr(N.dtcreated, DS_COMPACT), s.c_str(), N.jtype.c_str());
	}
	arJots.SetSortColumn(con.scol);
}

void Jotter::ARMenu(Bar &bar)
{
	int cur = arJots.GetCursor();
	bar.Add("Add jot", [this]{EditJot();}).Key(K_INSERT);
	if (cur>=0)
	{
		std::string st, s = arJots.Get(cur, 0).ToString().ToStd();
		size_t id = stot<size_t>(s);
		bar.Separator();
		s = arJots.Get(cur, 2).ToString().ToStd();
		s = s.substr(0, s.find('\n'));
		s = schop(s, 80); //arbitrary 80 chars max length
		TRIM(s);
		st = says("Show/Edit \"", s, "\"");
		bar.Add(st.c_str(), [id,this]{EditJot(id);}).Key(K_ENTER);
		bar.Separator();
		st = says("Delete \"", s, "\"");
		bar.Add(st.c_str(), [id,this]{DeleteJot(id);}).Key(K_DELETE);
	}
}

bool Jotter::add_jot(Jot &jot)
{
	if (DB.Save(jot))
	{
		JWords.AddData(jot.sJot, jot.idJot);
		jots.add(jot);
		FillArt();
		return true;
	}
	return false;
}

bool Jotter::add_jtype(const std::string &sn) //true if not exist
{
	bool b{false};
	if (!vjtypes.Has(sn)&&DB.SaveJType(sn)) { b=DB.LoadJTypes(vjtypes); }
	return b;
}

void Jotter::del_jtype(const std::string &sn)
{
	if (!DB.DeleteJType(sn)) sayfail("cannot delete '", sn, "' ", DB.GetLastError());
}

void Jotter::EditJot(size_t id)
{
	Jot J;
	if (id>0) { if (!jots.getjot(id, &J)) { sayerr("no such jot"); return; }}
	UIJot uij(this);
	uij.ebJot.SetData(J.sJot.c_str());
	for (auto s:vjtypes)
	{
		uij.dlType.Add(s.c_str());
	}
	if (!vjtypes.Has(J.jtype)) J.jtype="jot";
	uij.dlType.Set(J.jtype.c_str());
	uij.Execute();
	if (uij.bOK)
	{
		J.sJot = uij.ebJot.GetData().ToString().ToStd();
		J.jtype = uij.dlType.Get().ToString().ToStd();
		LTRIM(J.sJot);
		add_jot(J);
	}
}

void Jotter::DeleteJot(size_t id)
{
	Jot jot;
	
	if (!jots.getjot(id, &jot)) { sayerr("no such jot"); return; }
	DB.DeleteJot(id);
	jots.remove(id);
	DB.SaveHistory(jot);
	FillArt();
}

void Jotter::ShowJot(size_t id)
{
	if (!id)
	{
		int cur = arJots.GetCursor();
		if (cur>=0)
		{
			std::string sid = arJots.Get(cur, 0).ToString().ToStd();
			id = stot<size_t>(sid);
		}
	}
	if (id)	EditJot(id);
}

void Jotter::DoDropwords()
{
	UIDrops dlg(&JWords);
	dlg.Execute();
}

void Jotter::ShowHistory()
{
	UIHistory dlg(this);
	dlg.Execute();
}

bool Jotter::Key(dword key, int count)
{
	if (key==K_ESCAPE) { Minimize(); return true; }
	if (key==K_ENTER) { ShowJot(); return true; }
	return TopWindow::Key(key, count);
}


//===================================================================================================
UIJot::UIJot(Jotter *pj) : pJ(pj)
{
	CtrlLayout(*this, "Jot");
	CenterOwner().Sizeable();
	ebJot.Clear();
	bOK=false;
	btnAddType.WhenAction = [this]()
		{
			std::string sn=askuser("Name the jot-type");
			TRIM(sn);
			if (!sn.empty()) { if (pJ->add_jtype(sn)) { dlType.Add(sn.c_str()); dlType.Set(sn.c_str()); }}
		};
	btnDelType.WhenAction = [this]()
		{
			int i=dlType.GetIndex();
			std::string sn=dlType.Get().ToString().ToStd();
			if (!sn.empty()) { pJ->del_jtype(sn); dlType.Remove(i); }
		};
	btnOK.WhenAction = [this]{bOK=true;Close();};
	btnCancel.WhenAction = [this]{bOK=false;Close();};
}

bool UIJot::Key(dword key, int count)
{
	if (key==K_ESCAPE) { btnCancel.WhenAction(); return true; }
	if (key==K_CTRL_ENTER) { btnOK.WhenAction(); return true; }
	//if (key==K_CTRL_S) { --- will need to change design
	return false;
}


//===================================================================================================
UIDrops::UIDrops(JotWords *p) : pJW{p}
{
	CtrlLayout(*this, "Dropwords");
	Sizeable().CenterOwner();
	
	clDrops.ColumnMode().Columns(7);
	clDrops.WhenBar = [this](Bar &bar)
		{
			int x=clDrops.GetCursor();
			bar.Add("Add dropword..", [this](){DoAddDW();}).Key(K_INSERT);
			if (x>=0)
			{
				std::string s{};
				s=clDrops.Get(x).ToString().ToStd();
				bar.Add(says("Remove dropword '", s, "'").c_str(), [this, s](){DoDelDW(s);}).Key(K_DELETE);
			}
			bar.Separator();
			bar.Add("Close", [this](){ Break(); }).Key(K_ESCAPE);
		};
	fill_cldrops();
}

void UIDrops::fill_cldrops()
{
	clDrops.Clear();
	if (pJW) { for (auto p:pJW->mdrop) clDrops.Add(p.first.c_str()); }
}

void UIDrops::DoAddDW()
{
	//to_do();
	std::string w;
	w=askuser("New Dropword");
	TRIM(w);
	pJW->AddDropword(w);
	fill_cldrops();
}

void UIDrops::DoDelDW(const std::string &w)
{
	pJW->RemoveDropword(w);
	fill_cldrops();
}

bool UIDrops::Key(dword key, int count)
{
	if ((key==K_INSERT)||(key==K_ENTER)) { DoAddDW(); return true; }
	if (key==K_DELETE)
	{
		int x=clDrops.GetCursor();
		if (x>=0)
		{
			std::string s{};
			s=clDrops.Get(x).ToString().ToStd();
			DoDelDW(s);
			return true;
		}
	}
	if (key==K_ESCAPE) { Close(); return true; }
	return TopWindow::Key(key, count);
}

//===================================================================================================
UIHistory::~UIHistory() {}
UIHistory::UIHistory(Jotter *pj)
{
	CtrlLayout(*this, "Jotter History");
	pJ=pj;
	
	arH.WhenBar = [this](Bar &bar){ arhmenu(bar); };
	btnClose.WhenAction = [this](){ Close(); };
	
	fillarh();
}

void UIHistory::fillarh()
{
	Jots hist;
	arH.Clear();
	arH.AddColumn().HeaderTab().Hide(); //id
	arH.AddColumn("deleted");
	if (DB.LoadHistory(hist))
	{
		for (auto h:hist)  arH.Add((int)h.idJot, h.sJot);
	}
}

void UIHistory::arhmenu(Bar &bar)
{
	int cur = arH.GetCursor();
	bool b=(cur>=0);
	std::string st{}, s{};
	size_t id{0};
	if (b)
	{
		s=arH.Get(cur, 0).ToString().ToStd();
		id = stot<size_t>(s);
		s=arH.Get(cur, 1).ToString().ToStd();
		s = s.substr(0, s.find('\n'));
		s = schop(s, 80); //arbitrary 80 chars max length
		TRIM(s);
		st = says("Restore \"", s, "\"");
		bar.Add(b, st.c_str(), [id,this]{ Restore(id);});
		bar.Separator();
		st = says("Destroy \"", s, "\"");
		bar.Add(b, st.c_str(), [id,this]{ Destroy(id);});
		bar.Separator();
	}
	bar.Add("Close", [this](){ Break(); });
}

bool UIHistory::Restore(size_t id)
{
	if (pJ)
	{
		Jot jot;
		if (DB.LoadHistoryJot(id, jot))
		{
			if (pJ->add_jot(jot)) Destroy(id, false);
			return true;
		}
	}
	return false;
}

void UIHistory::Destroy(size_t id, bool bask)
{
	if ((bask&&(PromptOKCancel("Are you sure you want to [* permanently destroy] this deleted jot?")))||!bask)
	{
		DB.DeleteHistory(id);
		fillarh();
	}
}

