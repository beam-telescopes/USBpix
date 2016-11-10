#include "histogrammer.h"
#include <algorithm>
#include <iomanip>

void HitHistogrammer::dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2) {
	if(tot1 < 14) {
		hist[row][column]++;
		hits++;
	}

	if(tot2 < 14) {
		hist[row + 1][column]++;
		hits++;
	}
}


void TotHistogrammer::dataRecord(unsigned /*column*/, unsigned /*row*/, unsigned tot1, unsigned tot2) {
	if(tot1 < 15) {
		hist[tot1]++;
	}

	if(tot2 < 15) {
		hist[tot2]++;
	}
}

void TotHistogrammer2D::dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2) {
  if(tot1 < (unsigned int)tot_max) {
		hist[row][column][tot1]++;
	}

	if(tot2 < (unsigned int)tot_max) {
		hist[row + 1][column][tot2]++;
	}
}

void BCIDHistogrammer::dataHeader(int bcid, int /*lv1id*/, int /*flag*/) {
	hist[(msb << 10) | bcid]++;
}

void BCIDHistogrammer::serviceRecord(int code, int count) {
	if(code == 14) { //fixme
		msb = selectBits(count, 0, 3);
	}
}

void LV1IDHistogrammer::dataHeader(int /*bcid*/, int lv1id, int /*flag*/) {
	hist[lv1id]++;
}

void LV1IDHistogrammer::serviceRecord(int code, int /*count*/) {
	if(code == 14) { //fixme

	}
}

void LVL1Histogrammer::dataHeader(int bcid, int lv1id, int /*flag*/) {
	if (first_data || (lv1id_low != lv1id) || (bcid != next_bcid)) {
		first_data = false;
		lv1_int = 0;
	} else {
		lv1_int = (lv1_int + 1) % 16;
	}

	next_bcid = (bcid + 1) % 1024;
	lv1id_low = lv1id;
}

void LVL1Histogrammer::dataRecord(unsigned /*column*/, unsigned /*row*/, unsigned /*tot1*/, unsigned /*tot2*/) {
	hist[lv1_int]++;
}

void SrHistogrammer::serviceRecord(int code, int /*count*/) {
	hist[code]++; //fixme
}

void RegisterDecoder::addressRecord(int /*type*/, int address) {
	this->address = address;
}

void RegisterDecoder::valueRecord(int value) {
	registers.push_back({address, value});
	address = 0xFFFF;
}

const std::vector<std::pair<uint16_t, uint16_t>> &RegisterDecoder::getRegisters(void) {
	return registers;
}

void PixelRegisterDecoder::addressRecord(int /*type*/, int address) {
	this->address = address;
}

void PixelRegisterDecoder::valueRecord(int value) {
	registers.push_back({address, ~value});
	address = 0xFFFF;
}

const std::vector<std::pair<uint16_t, uint16_t>> &PixelRegisterDecoder::getRegisters(void) {
	return registers;
}

void RawFileWriter::dataHeader(int bcid, int lv1id, int flag) {
	buffers[current_channel-1] << "DH " << flag << ' ' << lv1id << ' ' << bcid << std::endl;
}

void RawFileWriter::addressRecord(int type, int address) {
	buffers[current_channel-1] << "AR " << type << ' ' << address << std::endl;
}

void RawFileWriter::valueRecord(int value) {
	buffers[current_channel-1] << "VR " << value << std::endl;
}

void RawFileWriter::serviceRecord(int code, int count) {
	buffers[current_channel-1] << "SR " << code << ' ' << count << std::endl;
}

void RawFileWriter::dataRecord(unsigned column, unsigned row, unsigned tot1, unsigned tot2) {
	buffers[current_channel-1] << "DR " << (column+1) << ' ' << (row+1) << ' ' << tot1 << ' ' << tot2 << std::endl;
}

void RawFileWriter::trigger(uint32_t trigger_number) {
	for(int c : channels) {
		buffers[c-1] << "TD 0 0 " << trigger_number << std::endl;
	}
}

void RawFileWriter::rawData(uint32_t data) {
	if(count > 0x100000) { // flush after 4MiB of raw data
		count = 0;
		flush();
	}
	count++;

	if(data>>31 == 0) { // FE data
		uint8_t channel = selectBits(data, 24, 8);
		if(channel > 0 && channel <= 8) {
			current_channel = channel;
			buffers[current_channel-1] << "0x" << std::hex << std::setw(6) << std::setfill('0') << (data & 0x00FFFFFF) << std::endl << std::dec;
		}
	} else { // Trigger data
		for(int c : channels) {
			buffers[c-1] << "0x" << std::hex << std::setw(8) << std::setfill('0') << (data & 0x7FFFFFFF) << std::endl << std::dec;
		}
	}
}

void RawFileWriter::flush(void) {
	for(int c : channels) {
		std::cout << "CHANNEL " << c << std::endl; 
		o << "CHANNEL " << c << std::endl;
		o << buffers[c-1].str();
		buffers[c-1].str("");
	}
}

template class Histogrammer1D<4096>; // explicit instantiation to make VC linker happy
template class Histogrammer1D<8192>;
template class Histogrammer1D<16>;
template class Histogrammer2D<336,80>;

