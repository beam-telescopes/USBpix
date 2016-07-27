#ifndef _DATADECODER_H
#define _DATADECODER_H

#include <vector>
#include <cstdint>
#include <functional>

#include "dllexport.h"

class DllExport Histogrammer;

class DllExport DataDecoder {
	public:
		DataDecoder(void) {}
		DataDecoder(std::initializer_list<Histogrammer*> il);

		void decode(const std::vector<uint32_t> &data);
		void addHistogrammer(Histogrammer *hist);
		void clearHistogrammers(void);

		static const int NUM_CHANNELS = 8;

		std::size_t size = 0;
		std::size_t invalid_dr = 0;
		std::size_t data_records = 0;
		std::size_t data_headers = 0;

	protected:
		void dataHeader(int channel, int bcid, int lv1id, int flag);
		void addressRecord(int channel, int type, int address);
		void valueRecord(int channel, int value);
		void serviceRecord(int channel, int code, int count);
		void dataRecord(int channel, unsigned column, unsigned row, unsigned tot1, unsigned tot2);

		void trigger(uint32_t trigger_number);
		void rawData(uint32_t data);

		std::vector<Histogrammer*> histogrammer[NUM_CHANNELS + 2]; //fixme
};

#endif
