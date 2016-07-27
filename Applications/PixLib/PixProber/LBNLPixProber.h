#ifndef _PIXLIB_LBNLPIXPROBER
#define _PIXLIB_LBNLPIXPROBER

#include "PixProber.h"

#include <iostream>
#include <fstream>

#ifdef WIN32
// windows serial stuff
#using <system.dll>
#include <gcroot.h>
using namespace System::IO::Ports;
#endif

namespace PixLib {
	
	class LBNLPixProber : public PixProber {
	
		public:
			
			LBNLPixProber(DBInquire *dbInquire=0);  
			~LBNLPixProber(); 
			
			// probe station initilization
			void initHW();

			// movement functions
			void chuckGotoFirst();
			void chuckContact();
			void chuckSeparate();
			bool chuckNextDie();
			void chuckGotoPosition(int col, int row, int type);
			void chuckGetPosition(int &col, int &row, int &type);
			void chuckGetIndex(int &index);
			void chuckGotoIndex(int index);
	
		private:
			
			 //! Init configuration structure
			void configInit();  
			
			// internal communication with the nclient functions
			void connectPS();
			void disconnectPS();

			std::string m_myIP;
			int m_port;
			bool m_notify;
			bool m_wasInit;


			// serial object
#ifdef WIN32
			gcroot<SerialPort^> serialPort;
#endif


	};

}

#endif
