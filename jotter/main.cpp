
#include "jotter.h"
#include "jdb.h"
#include <utilfuncs/utilfuncs.h>

//hack to get window-manager decoration sizes (title-bar-height, window-frame-thickness)
extern int WFRAMEWIDTH;//jotter.cpp
extern int WTITLEBARHEIGHT;
void get_wframe_wh()
{
	TopWindow w,w1;
	w.SetRect(10,10,1,1);
	w.OpenMain();
	w1.SetRect(w.GetRect());
	w1.OpenMain();
	Rect rw=w.GetRect();
	Rect rw1=w1.GetRect();
	WFRAMEWIDTH=rw1.left-rw.left;
	WTITLEBARHEIGHT=rw1.top-rw.top;
	w.Close();
	w1.Close();
}

const std::string DBNAME{"JotterDB.sqlite3"};
bool initjotter()
{
	std::string sdb{}, sp{};
	bool b;
	default_output_path(sp);
	sdb=path_append(sp, DBNAME);
	b=file_exist(sdb);
	if (DB.Open(sdb)) { if (!b) DB.ImplementSchema(); return true; }
	return sayerr(DB.GetLastError().c_str());
}

GUI_APP_MAIN
{
	get_wframe_wh();
	if (initjotter())
	{
		Jotter().Run();
	}
}
