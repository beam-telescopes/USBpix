/*
* 4 July 2012, Bonn, pohl@physik.uni-bonn.de
* This is a simple and fast clusterizer. With a run time that is linearily dependend from:
* _dx*_dy*_DbCID*number of hits
*   _dx,_dy are the x,y-step sizes in pixel to search around a hit for additional hits
*   _DbCID is the BCID window hits are clustered together
*   number of hits per trigger/event is usually <10
*
* The basic idea is:
* - use an array that is looped over only if hits are inside. Per trigger you have usually < 10 hits.
*   Methods: Clusterize() for looping over the array
* - start at one hit position and search around it with a distance of _dx,_dy (8 directions: up, up right, right ...) and _DbCID
* 	Methods: Clusterize() for looping over the hit array and calling SearchNextHits() for finding next hits belonging to the clusters
* - only increase the search distance in a certain direction (until _dx, _dy, _DbCID) if no hit was found in this direction already
*   Method: SearchNextHits() does this
* - do this iteratively and delete hits from the map if they are added to a cluster
*   Method: SearchNextHits() deletes hits from the hit map if they are assigned to a cluster
* - if the hit map is empty all hits are assigned to cluster, abort then
* 	Method: Clusterize() does this
*
* 	The clusterizer can either be filled externally with hits (addHit method) or it can extract the hits from the
* 	SRAM data including persistance check (clusterRawData method)
*/

#pragma once

#include <iostream>
#include <map>
#include <vector>

#include "defines.h"

#define __CLUSTERIZER_DEBUG false
#define __MAXTOTUSED 13
#define __BCIDCOUNTERSIZE_FEI4A 256	//BCID counter for FEI4A has 8 bit
#define __BCIDCOUNTERSIZE_FEI4B 1024//BCID counter for FEI4B has 10 bit

#include "dllexport.h"

class DllExport Clusterizer {

public:
	Clusterizer(void);
	~Clusterizer(void);

	bool clusterRawData(unsigned int pSRAMwordRB[]);

	bool clusterize();													//main function to start the clustering of the hit array

	void addHit(int pCol, int pRow, int pNbCID, int pTot, float pCharge = -1);	//add hit with column, row, relative BCID [0:15], tot, charge

	void setPixelChargeCalibration(unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge); //set for every pixel and every TOT [0:12] the measured charge
	void setXclusterDistance(unsigned int pDx);							//sets the x distance between two hits that they belong to one cluster
	void setYclusterDistance(unsigned int pDy);							//sets the x distance between two hits that they belong to one cluster
	void setBCIDclusterDistance(unsigned int pDbCID);					//sets the BCID depth between two hits that they belong to one cluster
	void setMinClusterHits(unsigned int pMinNclusterHits);				//minimum hits per cluster allowed, otherwise cluster omitted
	void setMaxClusterHits(unsigned int pMaxNclusterHits);				//maximal hits per cluster allowed, otherwise cluster omitted
	void setMaxClusterHitTot(unsigned int pMaxClusterHitTot);			//maximal tot for a cluster hit, otherwise cluster omitted
	void setMaxIncompleteEvents(unsigned int pMaxIncompleteEvents);		//maximal events with incomplete data structure for one SRAM read out leading to an abort of the clustering
	void setMaxWrongEvents(unsigned int pMaxWrongEvents);				//maximal events with wrong data structure for one SRAM read out leading to an abort of the clustering
	void setNbCIDs(int NbCIDs);											//set the number of BCIDs with hits for the actual trigger to save cluster time
	void setFEI4B(bool pIsFEI4B = true);								//set the FE flavor to be able to read the raw data correctly

	unsigned int getRunTime();											//returns a artificial number representing the needed computing time

	void clearHitMap();													//reset the hit map
	void clearClusterMaps();											//reset the cluster maps
	void resetRawDataCounters();										//reset the raw data counters (trigger number, BCID value,...)

	void showHits();													//shows the hit in the hit map for debugging
	void showHistsEntries();											//show the entries of the cluster histrogram for debugging

	//result arrays access functions
	unsigned long getClusterSizeOccurrence(int pClusterSize);			//returns how often the cluster size pClusterSize occurred
	unsigned long getClusterTotOccurrence(int pClusterTot);				//returns how often the cluster tot pClusterTot occurred
	unsigned long getClusterTotAndSizeOccurrence(int pClusterTot, int pClusterSize);		//returns how often the combination cluster tot pClusterTot/cluster size pClusterSize occurred
	unsigned long getClusterChargeAndSizeOccurrence(int pClusterCharge, int pClusterSize);	//returns how often the cluster charge pClusterCharge occurred
	unsigned long getClusterPositionOccurrence(int pClusterPosX, int pClusterPosY);	//returns how often the cluster position (pClusterPosX, pClusterPosY) occurred

private:
	inline void searchNextHits(int pCol, int pRow, int pBCID);			//search for a hit next to the actual one in time (BCIDs) and space (col, row)
	inline bool deleteHit(int pCol, int pRow, int pBCID);				//delete hit at position pCol,pRow from hit map, returns true if hit array is empty
	inline bool hitExists(int pCol, int pRow, int pBCID);				//check if the hit exists
	void initChargeCalibMap();											//sets the calibration map to all entries = 0
	void initHitMap();													//sets the hit map to no hit = all entries = -1
	void addClusterToResults();											//adds the actual cluster data to the result arrays
	void resetEventVariables();											//resets event variables before starting new event
	void printEventData();												//for debug purpose only

	bool getTimefromDataHeader(unsigned int& pSRAMWORD, unsigned int& pLVL1ID, unsigned int& pBCID);	//returns true if the SRAMword is a data header and if it is sets the BCID and LVL1
	bool getHitsfromDataRecord(unsigned int& pSRAMWORD, int& pColHit1, int& pRowHit1, int& pTotHit1, int& pColHit2, int& pRowHit2, int& pTotHit2);	//returns true if the SRAMword is a data record with reasonable hit infos and if it is sets pCol,pRow,pTot
	bool isTriggerWord(unsigned int& pSRAMWORD);						//returns true if data word is trigger word
	bool isOtherWord(unsigned int& pSRAMWORD);							//returns true if data word is an empty record, adress record, value record or service record

	void clearActualClusterData();

	bool _once;															//is set only for one event to have one example event data for the debug output

	//hit map for one event
	short int _hitMap[RAW_DATA_MAX_COLUMN][RAW_DATA_MAX_ROW][__MAXBCID];//array containing the hits TOT value for max 16 BCIDs
	float _chargeMap[RAW_DATA_MAX_COLUMN][RAW_DATA_MAX_ROW][14];		//array containing the lookup charge values for each pixel and TOT

	//cluster results
	unsigned long _clusterTots[__MAXTOTBINS][__MAXCLUSTERHITSBINS];		//array containing the cluster tots/cluster size for histogramming
	unsigned long _clusterCharges[__MAXCHARGEBINS][__MAXCLUSTERHITSBINS];//array containing the cluster tots/cluster size for histogramming
	unsigned long _clusterHits[__MAXCLUSTERHITSBINS];					//array containing the cluster number of hits for histogramming
	unsigned long _clusterPosition[__MAXPOSXBINS][__MAXPOSYBINS];		//array containing the cluster x positions for histogramming

	//cluster settings
	unsigned int _dx;													//max distance in x between to hits that they belong to a cluster
	unsigned int _dy;													//max distance in y between to hits that they belong to a cluster
	int _DbCID; 														//time window in BCIDs the clustering is done
	int _NbCID; 														//number of BCIDs for one trigger
	unsigned int _maxClusterHitTot; 									//the maximum number of cluster hit tot allowed, if exeeded cluster is omitted
	unsigned int _minClusterHits; 										//the minimum number of cluster hits allowed, if exeeded clustering aborted
	unsigned int _maxClusterHits; 										//the maximum number of cluster hits allowed, if exeeded clustering aborted
	unsigned int _runTime; 												//artificial value to represent the run time needed for clustering

	//actual clustering variables
	unsigned int _nHits;												//number of hits for the actual event data to cluster
	int _minColHitPos;													//minimum column with a hit for the actual event data
	int _maxColHitPos;													//maximum column with a hit for the actual event data
	int _minRowHitPos;													//minimum row with a hit for the actual event data
	int _maxRowHitPos;													//maximum row with a hit for the actual event data
	int _bCIDfirstHit;													//relative start BCID value of the first hit [0:15]
	int _bCIDlastHit;													//relative stop BCID value of the last hit [0:15]
	int _actualClusterTot;												//temporary value holding the total tot value of the actual cluster
	int _actualClusterHits;												//temporary value holding the total hit number of the actual cluster
	int _actualRelativeClusterBCID; 									//temporary value holding the relative BCID start value of the actual cluster [0:15]
	double _actualClusterX;												//temporary value holding the x position of the actual cluster
	double _actualClusterY;												//temporary value holding the y position of the actual cluster
	double _actualClusterCharge;										//temporary value holding the total charge value of the actual cluster

	bool _abortCluster;													//set to true if one cluster TOT hit exeeds _maxClusterHitTot, cluster is not added to the result array

	int tNdataHeader;													//number of data header per event
	int tNdataRecord;													//number of data records per event
	unsigned int tStartBCID;											//BCID value of the first hit for the event window
	unsigned int tStartLVL1ID;											//LVL1ID value of the first data header of the event window
	unsigned int tDbCID;												//relative BCID of on event window [0:15], counter
	bool tLVL1IDisConst;												//is only true if a trigger is send externally, self trigger can have different LVL1IDs in one event

	bool _abortClustering;												//not in use, can be used to abort clustering

	//variables for the raw data processing
	unsigned long _nTriggers;											//stores the total number of triggers counted for the source scan
	unsigned long _nEvents;												//stores the total number of valid events counted for the source scan
	unsigned long _nInvalidEvents;										//number of events with wrong data structure
	unsigned long _nIncompleteEvents;									//number of events with incomplete data structure
	unsigned long _maxInvalidEvents;									//maximum number of broken events for one SRAM read out accepted until clustering is aborted
	unsigned long _maxIncompleteEvents;									//maximum number of incomplete events for one SRAM read out accepted until clustering is aborted
	bool _fEI4B;														//set to true to distinguish between FE-I4B and FE-I4A
	bool tValidTriggerData;												//set to false if event data structure is strange to avoid clustering of invalid data
	bool tIncompleteEvent;												//set to true if event data is incomplete to omit the actual event for clustering
	unsigned long _nDataRecords;										//the total number of data records found
};
