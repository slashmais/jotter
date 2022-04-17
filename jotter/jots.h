#ifndef _jots_h_
#define _jots_h_

#include <string>
#include <vector>
#include <utilfuncs/utilfuncs.h>


struct Jot
{
	size_t idJot;
	std::string sJot;
	std::string jtype;
	DTStamp dtcreated;
	~Jot();
	Jot();
	Jot(const Jot &T) { (*this)=T; }
	Jot& operator=(const Jot &T) { idJot=T.idJot; sJot=T.sJot; jtype=T.jtype; dtcreated=T.dtcreated; return *this; }
	void clear();
	
};

struct Jots : public std::vector<Jot>
{
	~Jots();
	Jots();
	bool getjot(size_t id, Jot *pti=nullptr);
	void add(const Jot &N);
	void remove(size_t id);
};


#endif
