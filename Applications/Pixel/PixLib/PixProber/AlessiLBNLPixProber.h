#ifndef _PIXLIB_ALESSILBNLPIXPROBER
#define _PIXLIB_ALESSILBNLPIXPROBER

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
	
	class AlessiLBNLPixProber : public PixProber {
	
		public:
			//Set chip to chip pitch; units are in mm.
			double chip_pitch_x, chip_pitch_y;
			//std::ofstream myfile;
			AlessiLBNLPixProber(DBInquire *dbInquire=0);  
			~AlessiLBNLPixProber(); 
			
			// probe station initilization
			void initHW();

			// movement functions
			void Sleep(unsigned int mseconds);
			void chuckGotoFirst();
			void chuckContact();
			void chuckContactOnce();
			void chuckSeparate();
			bool chuckNextDie();
			void chuckGotoPosition(int col, int row, int type);
			void chuckGetPosition(int &col, int &row, int &type);
			void chuckGotoIndex(int index);
			void chuckGetIndex(int &index);
	
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
