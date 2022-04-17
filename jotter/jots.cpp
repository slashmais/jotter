#include "jots.h"

Jot::~Jot()						{}
Jot::Jot()						{ clear(); jtype="Jot"; dtcreated=dt_stamp(); }
void Jot::clear()				{ idJot=0; sJot.clear(); }

Jots::~Jots()					{ clear(); }
Jots::Jots()					{ clear(); }
void Jots::add(const Jot &N)	{ if (N.idJot) remove(N.idJot); push_back(N); }

bool Jots::getjot(size_t id, Jot *pN)
{
	for (auto t:(*this)) { if (t.idJot==id) { if (pN) { (*pN)=t; } return true; }}
	return false;
}

void Jots::remove(size_t id)
{
	auto it=begin();
	while (it!=end()) { if ((*it).idJot==id) { erase(it); return; } else it++; }
}

