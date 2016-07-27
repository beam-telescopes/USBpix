#ifndef DATASTRUCTI3_H
#define DATASTRUCTI3_H

#include <string.h>

struct ConfDataStruct_I3 {
#ifndef FX // application implementaion
	int index;
	char name[32];
#endif
	int add;
	int size;
	int value;
	void Init(ConfDataStruct_I3 data)
	{
#ifdef CF__LINUX
		strcpy(name, data.name);
#else
		strcpy_s(name, data.name);
#endif
		add = data.add;
		size = data.size;
		value = data.value;
	}
};

typedef ConfDataStruct_I3 ConfDataStructI3;
#endif
