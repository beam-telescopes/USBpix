#include <algorithm>

#include "datadecoder.h"
#include "histogrammer.h"
#include "util.h"

const uint8_t header = 0xE8; //0b11101 000

const uint8_t data_header = header | 0x1; //0b11101 001
const uint8_t address_record = header | 0x2; //0b11101 010
const uint8_t value_record = header | 0x4; //0b11101 100
const uint8_t service_record = header | 0x7; //0b11101 111

DataDecoder::DataDecoder(std::initializer_list<Histogrammer*> il) {
	for(Histogrammer *i : il) {
		if(i->channel >= -1 && i->channel < NUM_CHANNELS) {
			histogrammer[i->channel + 1].push_back(i);
		}
	}
}

void DataDecoder::decode(const std::vector<uint32_t> &data, std::function<void(uint32_t)> storedata_function) {
	size += data.size();
	for(auto i : data) {
		uint8_t channel = selectBits(i, 24, 8);

		if(storedata_function) storedata_function(i);
		rawData(i);

		if(channel >> 7) { //Trigger
			uint32_t trigger_number = selectBits(i, 0, 31); //testme
			trigger(trigger_number);
		} else {
			uint8_t type = selectBits(i, 16, 8);

			switch(type) {
				case data_header: {
					int bcid = selectBits(i, 0, 10);
					int lv1id = selectBits(i, 10, 5);
					int flag = selectBits(i, 15, 1);

					dataHeader(channel, bcid, lv1id, flag);
					data_headers++;

					break;
				}

				case address_record: {
					int type = selectBits(i, 15, 1);
					int address = selectBits(i, 0, 15);

					addressRecord(channel, type, address);
					break;
				}

				case value_record: {
					int value = selectBits(i, 0, 16);

					valueRecord(channel, value);
					break;
				}

				case service_record: {
					int code = selectBits(i, 10, 6);
					int count = selectBits(i, 0, 10);

					serviceRecord(channel, code, count);
					break;
				}

				default: { //data record
					unsigned tot2 = selectBits(i, 0, 4);
					unsigned tot1 = selectBits(i, 4, 4);
					unsigned row = selectBits(i, 8, 9) - 1;
					unsigned column = selectBits(i, 17, 7) - 1;

					if(column < 80 && ((tot2 == 15 && row < 336) || (tot2 < 15 && row < 335) )) {
						dataRecord(channel, column, row, tot1, tot2);
						data_records++;
					} else { // invalid data record
						invalid_dr ++;
					}

					break;
				}
			}
		}
	}
}

void DataDecoder::dataHeader(int channel, int bcid, int lv1id, int flag) {
	for(Histogrammer *hist : histogrammer[channel + 1]) {
		hist->dataHeader(bcid, lv1id, flag);
	}

	if(channel != -1) {
		dataHeader(-1, bcid, lv1id, flag);
	}
}

void DataDecoder::addressRecord(int channel, int type, int address) {
	for(Histogrammer *hist : histogrammer[channel + 1]) {
		hist->addressRecord(type, address);
	}

	if(channel != -1) {
		addressRecord(-1, type, address);
	}
}

void DataDecoder::valueRecord(int channel, int value) {
	for(Histogrammer *hist : histogrammer[channel + 1]) {
		hist->valueRecord(value);
	}

	if(channel != -1) {
		valueRecord(-1, value);
	}
}

void DataDecoder::serviceRecord(int channel, int code, int count) {
	for(Histogrammer *hist : histogrammer[channel + 1]) {
		hist->serviceRecord(code, count);
	}

	if(channel != -1) {
		serviceRecord(-1, code, count);
	}
}

void DataDecoder::dataRecord(int channel, unsigned column, unsigned row, unsigned tot1, unsigned tot2) {
	for(Histogrammer *hist : histogrammer[channel + 1]) {
		hist->dataRecord(column, row, tot1, tot2);
	}

	if(channel != -1) {
		dataRecord(-1, column, row, tot1, tot2);
	}
}

void DataDecoder::trigger(uint32_t trigger_number) {
	for(Histogrammer *hist : histogrammer[0]) {
		hist->trigger(trigger_number);
	}
}

void DataDecoder::rawData(uint32_t data) {
	for(Histogrammer *hist : histogrammer[0]) {
		hist->rawData(data);
	}
}

void DataDecoder::addHistogrammer(Histogrammer *hist) {
	if(hist->channel >=-1 && hist->channel < NUM_CHANNELS) {
		histogrammer[hist->channel + 1].push_back(hist);
	}
}

void DataDecoder::clearHistogrammers(void) {
	for(int i=0; i<NUM_CHANNELS + 1; i++) {
		histogrammer[i].clear();
	}
}
