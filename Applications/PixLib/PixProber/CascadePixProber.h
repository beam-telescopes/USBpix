/////////////////////////////////////////////////////////////////
//// CascadePixProber.h
//// Declaration for CascadePixProber Class
//// Andrew G Stewart
//// University of Glasgow
//// 30 August 2012
//////////////////////////////////////////////////////////////////

#ifndef _PIXLIB_CASCADEPIXPROBER
#define _PIXLIB_CASCADEPIXPROBER

#include "PixProber.h"

namespace PixLib {

	class CascadePixProber : public PixProber {

	public:

		// Default Constructor
		CascadePixProber(DBInquire *dbInquire=0);

		// Default Destructor
		~CascadePixProber();

		void initHW(); // member function
		void GoHome(); // member function - locates centre
		int GetDevice(); // member function - returns m_device (device descriptor from ibdev)

		// movement functions
		void chuckGotoFirst();
		void chuckContact();
		void chuckSeparate();
		bool chuckNextDie();
		void chuckGotoPosition(int col, int row, int type);
		void chuckGetPosition(int &col, int &row, int &type);
		void chuckGetIndex(int &index);
		void chuckGotoIndex(int index);
		
		int m_idnumber; // probe station identification number
		int m_device; // member variable - device descriptor	
		bool m_contact; // member variable - true if probes are in contact
	
	private:

		void configInit(); // Init configuration structure
	
		void connectPS(); // private member function
		void disconnectPS();
		void runProberCommand(const char *command, char *response);

		// additional member variables used in SuessPixProber
		int m_boardId;
		int m_GPIB_addr;
		bool m_wasInit;
	};

}
#endif
