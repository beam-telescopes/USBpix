#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <string.h>


struct ConfDataStruct_ {
#ifndef   FX   // application implementaion
	int			   regindex;
	int            index;
	char           name[32];
#endif
	unsigned short add;
	unsigned char  size;
	unsigned int value;
	bool inv_bitorder;
	void Init(ConfDataStruct_ data){
#ifdef CF__LINUX
		strcpy(name, data.name);
#else
		strcpy_s(name, data.name);
#endif
		index = data.index;
		add   = data.add;
		size  = data.size;
		value = data.value;
		inv_bitorder = data.inv_bitorder;
	}


};

typedef ConfDataStruct_ ConfDataStruct;
#endif

