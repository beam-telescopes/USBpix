#include "GRegister.h"

GRegister::GRegister(void)
{
	name = "";
	number = -1;
	address = 0;
	bitmask = 0x0000;
	defvalue = 0;

}
GRegister::GRegister(QString regname, int regnum, unsigned long regadd, unsigned regbitmask,int regdefvalue)
{
	name = regname;
	number = regnum;
	address = regadd;
	bitmask = regbitmask;
	defvalue = regdefvalue;
}


GRegister::~GRegister(void)
{
}

void GRegister::addGRegister(QString regname, int regnum, unsigned long regadd, unsigned regbitmask,int regdefvalue)
{
	name = regname;
	number = regnum;
	address = regadd;
	bitmask = regbitmask;
	defvalue = regdefvalue;
}
