#ifndef _HISTOGRAMMER_H
#define _HISTOGRAMMER_H

#include <array>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <cstdint>

#include "util.h"
#include "dllexport.h"

#include <iostream>

class DllExport Histogrammer {
	public:
		Histogrammer(int _channel = -1) : channel(_channel) {}
		virtual ~Histogrammer(void) {}

		virtual void dataHeader(int /*bcid*/, int /*lv1id*/, int /*flag*/) {}
		virtual void addressRecord(int /*type*/, int /*address*/) {}
		virtual void valueRecord(int /*value*/) {}
		virtual void serviceRecord(int /*code*/, int /*count*/) {}
		virtual void dataRecord(unsigned /*column*/, unsigned /*row*/, unsigned /*tot1*/, unsigned /*tot2*/) {}

		virtual void trigger(uint32_t /*trigger_number*/) {}
		virtual void rawData(uint32_t /*data*/) {}

		const int channel;
	protected:
};

template <size_t NUM>
class DllExport Histogrammer1D : public Histogrammer {
	public:
		Histogrammer1D(int _channel = -1) : Histogrammer(_channel) {hist = {{0}};} // VC12 does not support inheriting constructors
	
		const std::size_t & operator[](std::size_t i) {
			return hist[i];
		}

		const size_t size = NUM;

		friend std::ostream &operator<<(std::ostream &o, const Histogrammer1D<NUM> &hist) {
			for(size_t i = 0; i < NUM; i++) {
				o << i << ' ' << hist.hist[i] << std::endl;
			}

			return o;
		}

	protected:
		std::array<std::size_t, NUM> hist;// = {{0}};
};


template <size_t ROW, size_t COL>
class DllExport Histogrammer2D : public Histogrammer {
	public:
		Histogrammer2D(int _channel = -1) : Histogrammer(_channel) {
                  hist = {{{{0}}}};
                }

		const std::array<std::size_t, COL> & operator[](std::size_t i) {
			return hist[i];
		}

		const size_t rows = ROW;
		const size_t columns = COL;

		friend std::ostream &operator<<(std::ostream &o, const Histogrammer2D<ROW, COL> &hist) {
			for(size_t row = 0; row < ROW; row++) {
				for(size_t col = 0; col < COL; col++) {
					o << hist.hist[row][col] << ' ';
				}
				o << std::endl;
			}
			return o;
		}

	protected:
		Matrix<std::size_t, ROW, COL> hist;// = {{{{0}}}};
};

class DllExport HitHistogrammer : public Histogrammer2D<336, 80> {
	public:
		HitHistogrammer(int _channel = -1) : Histogrammer2D(_channel) {}

		void dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2);

		std::size_t hits = 0;
};

class DllExport TotHistogrammer : public Histogrammer1D<16> {
	public:
		TotHistogrammer(int _channel = -1) : Histogrammer1D(_channel) {}

		void dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2);
};

class DllExport TotHistogrammer2D : public Histogrammer {
	public:
		TotHistogrammer2D(int _channel = -1, int _tot_max = 15) : Histogrammer(_channel), tot_max(_tot_max) {
                  //hist = {{{{{{0}}}}}}; // somehow not supported by VS2013: causes a crash! so let's try the old-fashioned way...
				  for(int i=0; i<16; i++){
					for(int j=0; j<336; j++){
						for(int k=0; k<80; k++){
							hist[j][k][i] = 0;
						}
					}
				  }
                }

		const std::array<std::array<std::size_t, 16>, 80> & operator[](std::size_t i) {
			return hist[i];
		}

		void dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2);

		const size_t rows = 336; //fixme
		const size_t columns = 80;
		const int tot_max;

	protected:
		Matrix<std::array<std::size_t, 16>, 336, 80> hist;// = {{{{{{0}}}}}};
};

class DllExport BCIDHistogrammer : public Histogrammer1D<8192> {
	public:
		BCIDHistogrammer(int _channel = -1) : Histogrammer1D(_channel) {}

		void dataHeader(int bcid, int lv1id, int flag);
		void serviceRecord(int code, int count);

	protected:
		uint8_t msb = 0;
};

class DllExport LV1IDHistogrammer : public Histogrammer1D<4096> {
	public:
		LV1IDHistogrammer(int _channel = -1) : Histogrammer1D(_channel) {}

		void dataHeader(int bcid, int lv1id, int flag);
		void serviceRecord(int code, int count);

	protected:
		uint8_t msb = 0;
};

class DllExport LVL1Histogrammer : public Histogrammer1D<16> {
	public:
		LVL1Histogrammer(int _channel = -1) : Histogrammer1D(_channel) {}

		void dataHeader(int bcid, int lv1id, int flag);
		void dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2);

	protected:
		bool first_data = true;
		int next_bcid;
		int lv1id_low;
		int lv1_int;
};

class DllExport SrHistogrammer : public Histogrammer1D<32> {
	public:
		SrHistogrammer(int _channel = -1) : Histogrammer1D(_channel) {}

		void serviceRecord(int code, int count);
};

class DllExport RegisterDecoder : public Histogrammer {
	public:
		RegisterDecoder(int _channel = -1) : Histogrammer(_channel) {}

		void addressRecord(int type, int address);
		void valueRecord(int value);
		const std::vector<std::pair<uint16_t, uint16_t>> &getRegisters(void);

	protected:
		std::vector<std::pair<uint16_t, uint16_t>> registers;
		uint16_t address = 0xFFFF;
};

class DllExport PixelRegisterDecoder : public Histogrammer {
	public:
		PixelRegisterDecoder(int _channel = -1) : Histogrammer(_channel) {}

		void addressRecord(int type, int address);
		void valueRecord(int value);
		const std::vector<std::pair<uint16_t, uint16_t>> &getRegisters(void);

	protected:
		std::vector<std::pair<uint16_t, uint16_t>> registers;
		uint16_t address = 0xFFFF;
};

/* class DllExport RawFileWriter : public Histogrammer { */
/* 	public: */
/* 		RawFileWriter(const std::vector<int> &_channels, const std::string &_file) : channels(_channels), o(_file) { } */
/* 		~RawFileWriter(void) { flush(); } */

/* 		void dataHeader(int bcid, int lv1id, int flag); */
/* 		void addressRecord(int type, int address); */
/* 		void valueRecord(int value); */
/* 		void serviceRecord(int code, int count); */
/* 		void dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2); */

/* 		void trigger(uint32_t trigger_number); */
/* 		void rawData(uint32_t data); */

/* 	protected: */
/* 		void flush(void); */

/* 		int current_channel; */
/* 		std::array<std::ostringstream, 8> buffers; */
/* 		std::vector<int> channels; */
/* 		std::ofstream o; */

/* 		std::size_t count = 0; */
/* }; */

class DllExport RawFileWriter : public Histogrammer {
	public:
		RawFileWriter(const std::vector<int> &_channels, const std::string &_file){ }
		~RawFileWriter(void) { flush(); }

		void dataHeader(int bcid, int lv1id, int flag){};
		void addressRecord(int type, int address){};
		void valueRecord(int value){};
		void serviceRecord(int code, int count){};
		void dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2){};

		void trigger(uint32_t trigger_number){};
		void rawData(uint32_t data){};
		void flush(void){};
};

#endif
