#include <Suess.h>
#include <memory>
#include <iostream>

using namespace Suess;

void main(int argc, char **argv)
{
	std::auto_ptr<ProbeStation> ps (new ProbeStation("10.76.86.22", 1000, "TestProber", false, -1));
	ps->MoveChuckContact();
	ps->MoveChuckSeparation();
	ps->MoveChuck(10000,10000,'R','Y',1,'D');
	ps->MoveChuck(-10000,-10000,'R','Y',1,'D');
	ps->ReadChuckPosition();
	//std::cout <<
}