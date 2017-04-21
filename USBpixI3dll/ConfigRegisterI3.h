#ifndef CONFIGREGISTERI3_H
#define CONFIGREGISTERI3_H

#ifdef CF__LINUX
#include <sys/time.h>
#endif

#ifdef __VISUALC__
#include "stdafx.h"
#endif

#include <iostream>
#include <fstream> 
#include <iomanip>
#include <list>

#include "definesI3.h"
#include "SiLibUSB.h"
#include "dllexport.h"

class DllExport ConfigRegisterI3
{
public:
	ConfigRegisterI3(SiUSBDevice* Handle);
//	~ConfigRegisterI3();
	void SetUSBHandle(SiUSBDevice* Handle);
	void ResetFPGA();
	void StartMeasurement();
	void StopMeasurement();
	void PauseMeasurement();
	void ResumeMeasurement();
	void SetNumberOfEvents(int data);
	void WriteSync(int length);
	int ReadRegister(int CS);
	void WriteRegister(int CS, int data);
	void SetCalibrationMode();
	void SetRunMode();
	void SetTOTMode();
	void SetFakeInjectionMode();
	void ReadSRAM();
	void ReadSRAM(int StartAdd, int NumberOfWords);
	void ClearSRAM();
	void WriteSRAM(int StartAdd, int NumberOfWords);
	bool WriteSRAMWords(char* filename);
	bool WriteSRAMBitsFromWords(char *filename);
	bool WriteSRAMBytes(char* filename);
	bool WriteSRAMBitsFromBytes(char *filename);
	void ResetSRAMCounter();
	void SetSRAMCounter(int startadd);
	bool WriteTurboDAQFromRawData(const char* filename, bool new_file, bool close_file);
	void ClearHitTOTHistoFromRawData();
	void ClearHitHistoFromRawData();
	void ClearHitLV1HistoFromRawData();
	void ClearClusterSizeHistoFromRawData();
	void ClearClusterTOTHistoFromRawData();
	void ClearClusterSeedTOTHistoFromRawData();
	void ClearClusterSeedLV1HistoFromRawData();
	void ClearClusterSeedHistoFromRawData();
	void ClearClusterSeedPerTriggerHistoFromRawData();
	void ClearConfHisto();
	void ClearTOTHisto();
	void MakeTOTHisto();
	void MakeConfHisto();
	void MakeHitTOTHistoFromRawData();
	void MakeHitHistoFromRawData();
	void MakeHitLV1HistoFromRawData();
	void MakeClusterHistoFromRawData(int max_cluster_col_distance, int max_cluster_row_distance, int max_cluster_depth, int timewalk_threshold_max, bool enable_advanced_clustering);
	void GetConfHisto(int col, int row, int confstep, int& Value);
	void GetTOTHisto(int col, int row, int tot, int& Value);
	void GetRawData(std::list<unsigned int>& Data);
	void GetHitTOTHistoFromRawData(int col, int row, int TOT, int& Value);
	void GetHitHistoFromRawData(int col, int row, int& Value);
	void GetHitLV1HistoFromRawData(int LV1ID, int& Value);
	void GetClusterSizeHistoFromRawData(int Size, int& Value);
	void GetClusterTOTHistoFromRawData(int TOT, int Size, int& Value);
	void GetClusterSeedTOTHistoFromRawData(/*int col, int row, */int TOT, int Size, int& Value);
	void GetClusterSeedLV1HistoFromRawData(int LV1ID, int Size, int& Value);
	void GetClusterSeedHistoFromRawData(int col, int row, int Size, int& Value);
	void GetClusterSeedPerTriggerHistoFromRawData(int Amount, int& Value);
	void GetuCScanStatus(int& MaskStep, bool& ScanBusy, bool& ScanReady, bool& ScanCanceled, bool& ScanError, bool& FEI3Warning, bool& FEI3Error1, bool& FEI3Error2, bool& FEI3Error3, bool& FEI3Error4);
	void GetSourceScanStatus(bool& SRAMReadoutReadySignal, bool& SRAMFullSignal, bool& MeasurementPauseSignal, bool& MeasurementRunningSignal, int& SRAMFillLevel, int& NumberOfTriggers, int& NumberOfHitWords, int& TriggerRate, int& HitRate);

private:
	SiUSBDevice* myUSB;
	void BuildWords();
	void ResetSRAMAddressFSM();
	bool HitIsSeed(int seed_col, int seed_row, int seed_eoe, int max_cluster_depth, int col_start = 0, int col_end = 17, int row_start = 0, int row_end = 159);
	bool FindNextSeed(int& seed_col, int& seed_row, int eoe, int max_cluster_depth, int col_start = 0, int col_end = 17, int row_start = 0, int row_end = 159);
	void Clustering(int max_cluster_col_distance, int max_cluster_row_distance, int max_cluster_depth, int timewalk_threshold_max, bool enable_advanced_clustering, int& bcid_window_cluster_counter);
	void SimpleClustering(int seed_col, int seed_row, int seed_eoe, int max_cluster_col_distance, int max_cluster_row_distance, int max_cluster_depth, int timewalk_threshold_max, int bcid_window_cluster_number);
	void AdvancedClustering(int seed_col, int seed_row, int seed_eoe, int eoe_start, int max_cluster_col_distance, int max_cluster_row_distance, int max_cluster_depth, int timewalk_threshold_max, int bcid_window_cluster_number);
	bool Collision(int eoe, int bcid_window_cluster_number);
	void SeedNeighbours(int seed_col, int seed_row, int seed_eoe, int eoe_start, int max_cluster_depth, int& cluster_size, int& cluster_tot);

	unsigned char SRAMdata[SRAM_BYTESIZE]; // write buffer
	unsigned char SRAMdataRB[SRAM_BYTESIZE]; // read buffer
	unsigned int SRAMwordsRB[SRAM_WORDSIZE]; // read buffer, 32-bit words

	int ConfHisto[18][160][256];
	int TOTHisto[18][160][256];

	int HitHistoFromRawData[18][160];
	int HitTOTHistoFromRawData[18][160][256];
	int HitLV1HistoFromRawData[16];
	int HitLV1HistoTurboDAQ[16];

	int ClusterSizeHistoFromRawData[16];
	int ClusterTOTHistoFromRawData[256][16];
	int ClusterSeedTOTHistoFromRawData/*[18][160]*/[256][16];
	int ClusterSeedLV1HistoFromRawData[16][16];
	int ClusterSeedHistoFromRawData[18][160][16];
	int ClusterSeedPerTriggerHistoFromRawData[16];

	int bcid_window_tot_array[18][160][16];
	bool bcid_window_hit_array[18][160][16];
	int bcid_window_hit_cluster_number[18][160][16];
	int bcid_window_seed_cluster_number[18][160][16];
	int bcid_window_hit_collision_cluster_number[18][160][16];
	bool seed_neighbours_hit_processed_array[18][160][16];

	int total_trigger_counter;
	int total_valid_bcid_window;
	int	total_no_hit_counter;
	int total_hit_counter;
	int total_eoe_counter;
	int length_LV1;
	int total_processing_time;

	bool isCalMode;
	bool isTOTMode;

	time_t start_time;
	time_t stop_time;
};

#endif // CONFIGREGISTER_H
