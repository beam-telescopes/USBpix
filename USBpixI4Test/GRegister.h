#pragma once
#include <QtGui>

class GRegister
{
public:
	GRegister(void);
	GRegister(QString regname, int regnum, unsigned long regadd, unsigned regbitmask, int regdefvalue);
	~GRegister(void);

	void addGRegister(QString regname, int regnum, unsigned long regadd, unsigned regbitmask, int regdefvalue);
	int getNumber(){ return number;}
	QString getName(){ return name;}
	unsigned getMask(){return bitmask;}
	unsigned long getAddress(){return address;}
	int getValue(){return defvalue;}
	void setValue(int value){ defvalue = value;}

private:
	QString name;
	int number;
	unsigned long address;
	unsigned bitmask;
	int defvalue;

};
