// RenameWaferFiles.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <string>
using namespace std;


string convertInt(int number)
{
   stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}

void main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++)
	{
		cout << "arg: " << i << " \t argument: " << argv[i] << endl;
	}

	if (argc != 2)
	{
		cout << "set FileGroupName as argument" << endl << endl;
		return;
	}

	string FileGroupName = "";

	for (int i = 0; i < argc; i++)
	{
		if (i == 1)
			FileGroupName += argv[i];
	}

	string oldfilename = FileGroupName;
	string newfilename = FileGroupName;

	for (int i = 1; i <= 60; i++)
	{
		oldfilename = FileGroupName;
		newfilename = FileGroupName;
		switch (i)
		{
		case 1:
			oldfilename += "_-2_6.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 2:
			oldfilename += "_-2_5.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 3:
			oldfilename += "_-2_4.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 4:
			oldfilename += "_-2_3.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 5:
			oldfilename += "_-2_2.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 6:
			oldfilename += "_-1_7.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 7:
			oldfilename += "_-1_6.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 8:
			oldfilename += "_-1_5.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 9:
			oldfilename += "_-1_4.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 10:
			oldfilename += "_-1_3.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 11:
			oldfilename += "_-1_2.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 12:
			oldfilename += "_-1_1.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 13:
			oldfilename += "_0_8.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 14:
			oldfilename += "_0_7.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 15:
			oldfilename += "_0_6.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 16:
			oldfilename += "_0_5.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 17:
			oldfilename += "_0_4.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 18:
			oldfilename += "_0_3.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 19:
			oldfilename += "_0_2.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 20:
			oldfilename += "_0_1.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 21:
			oldfilename += "_0_0.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 22:
			oldfilename += "_1_8.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 23:
			oldfilename += "_1_7.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 24:
			oldfilename += "_1_6.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 25:
			oldfilename += "_1_5.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 26:
			oldfilename += "_1_4.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 27:
			oldfilename += "_1_3.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 28:
			oldfilename += "_1_2.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 29:
			oldfilename += "_1_1.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 30:
			oldfilename += "_1_0.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 31:
			oldfilename += "_2_8.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 32:
			oldfilename += "_2_7.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 33:
			oldfilename += "_2_6.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 34:
			oldfilename += "_2_5.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 35:
			oldfilename += "_2_4.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 36:
			oldfilename += "_2_3.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 37:
			oldfilename += "_2_2.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 38:
			oldfilename += "_2_1.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 39:
			oldfilename += "_2_0.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 40:
			oldfilename += "_3_8.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 41:
			oldfilename += "_3_7.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 42:
			oldfilename += "_3_6.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 43:
			oldfilename += "_3_5.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 44:
			oldfilename += "_3_4.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 45:
			oldfilename += "_3_3.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 46:
			oldfilename += "_3_2.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 47:
			oldfilename += "_3_1.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 48:
			oldfilename += "_3_0.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 49:
			oldfilename += "_4_7.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 50:
			oldfilename += "_4_6.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 51:
			oldfilename += "_4_5.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 52:
			oldfilename += "_4_4.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 53:
			oldfilename += "_4_3.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 54:
			oldfilename += "_4_2.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 55:
			oldfilename += "_4_1.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 56:
			oldfilename += "_5_6.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 57:
			oldfilename += "_5_5.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 58:
			oldfilename += "_5_4.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 59:
			oldfilename += "_5_3.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		case 60:
			oldfilename += "_5_2.root";
			newfilename += "_" + convertInt(i) + ".root";
			rename(oldfilename.c_str(), newfilename.c_str());
			cout << "renamed " << oldfilename << " to " << newfilename << endl;
			break;
		default:
			break;
		}
	}
	return;
}

