#ifdef CF__LINUX
#elif defined __VISUALC__
#include "StdAfx.h"
#endif

#include "Clusterizer.h"

Clusterizer::Clusterizer(void)
{
	_dx = 1;
	_dy = 1;
	_DbCID = 0;
	_minClusterHits = 1;
	_maxClusterHits = 9;	//std. setting for maximum hits per cluster allowed
	_runTime = 0;
	_nHits = 0;
	_fEI4B = true;
	_nTriggers = 0;
	_nEvents = 0;
	_once = true;			//show more clustering infos for the first BCID window
	_maxInvalidEvents = 5;
	_maxIncompleteEvents = 50;
	_nInvalidEvents = 0;
	_maxClusterHitTot = 13;
	_nIncompleteEvents = 0;
	_minColHitPos = RAW_DATA_MAX_COLUMN-1;
	_maxColHitPos = 0;
	_minRowHitPos = RAW_DATA_MAX_ROW-1;
	_maxRowHitPos = 0;
	_abortClustering = false;
	tStartBCID = 0;
	tStartLVL1ID = 0;
	tLVL1IDisConst = true;
	initChargeCalibMap();
	resetEventVariables();
	initHitMap();
	clearClusterMaps();
}
Clusterizer::~Clusterizer(void)
{
}

bool Clusterizer::clusterRawData(unsigned int pSRAMwordRB[])
{
	if (__CLUSTERIZER_DEBUG) std::cout<<"Clusterizer::clusterRawData"<<std::endl;

	_nInvalidEvents = 0;
	_nIncompleteEvents = 0;

	//start in defined condition
	clearHitMap();
	clearActualClusterData();
	resetEventVariables();

	//temporary variables set according to the actual SRAM word
	unsigned int tActualLVL1ID = 0;										//LVL1ID of the actual data header
	unsigned int tActualBCID = 0;										//BCID of the actual data header
	int tActualCol1 = 0;												//column position of the first hit in the actual data record
	int tActualRow1 = 0;												//row position of the first hit in the actual data record
	int tActualTot1 = -1;												//tot value of the first hit in the actual data record
	int tActualCol2 = 0;												//column position of the second hit in the actual data record
	int tActualRow2 = 0;												//row position of the second hit in the actual data record
	int tActualTot2 = -1;												//tot value of the second hit in the actual data record

	_once = true;														//detailed debug output for the first cluster, set to false later

	for (int iWord = 0; iWord < (int) SRAM_WORDSIZE; ++iWord){			//loop over the SRAM words
		unsigned int tActualWord = pSRAMwordRB[iWord];					//take the actual SRAM word
		tActualTot1 = -1;												//TOT1 value stays negative if it can not be set properly in getHitsfromDataRecord()
		tActualTot2 = -1;												//TOT2 value stays negative if it can not be set properly in getHitsfromDataRecord()
		if (getTimefromDataHeader(tActualWord, tActualLVL1ID, tActualBCID)){	//data word is data header if true is returned
			if (tNdataHeader > _NbCID-1){	//maximum event window is reached (tNdataHeader > BCIDs, mostly tNdataHeader > 15) and no trigger word occurred --> FE self trigger scan, so cluster data now
				if(__CLUSTERIZER_DEBUG & _once) printEventData();
				if(!tValidTriggerData)
					_nInvalidEvents++;
				else if (_bCIDfirstHit != -1){							//_bCIDfirstHit == -1 if no data record thus no hit exists in the event window
					if (!clusterize()){
						if(__CLUSTERIZER_DEBUG) std::cout<<"Clusterizer::clusterRawData: FAILED TO CLUSTER SELF TRIGGER DATA"<<std::endl;
						return false;
					}
				}

				_nEvents++;
				_once = false;
				resetEventVariables();
			}
			if (tNdataHeader == 0){								//set the BCID of the first data header
				tStartBCID = tActualBCID;
				tStartLVL1ID = tActualLVL1ID;
			}
			else{
				tDbCID++;										//increase relative BCID counter [0:15]
				if(_fEI4B){
					if(tStartBCID + tDbCID > __BCIDCOUNTERSIZE_FEI4B-1)	//BCID counter overflow for FEI4B (10 bit BCID counter)
						tStartBCID = tStartBCID - __BCIDCOUNTERSIZE_FEI4B;
				}
				else{
					if(tStartBCID + tDbCID > __BCIDCOUNTERSIZE_FEI4A-1)	//BCID counter overflow for FEI4A (8 bit BCID counter)
						tStartBCID = tStartBCID - __BCIDCOUNTERSIZE_FEI4A;
				}
				if (tStartBCID+tDbCID != tActualBCID) 			//check if BCID is increasing in the event window
					tIncompleteEvent = true;					//BCID not increasing, abort event and take actual data header for the first hit of the new event
				if (tActualLVL1ID != tStartLVL1ID)
					tLVL1IDisConst = false;
			}
			if(__CLUSTERIZER_DEBUG & _once) std::cout<<iWord<<" DH LVL1ID/BCID "<<tActualLVL1ID<<"/"<<tActualBCID<<"\n";
			tNdataHeader++;										//increase data header counter
		}
		else if (getHitsfromDataRecord(tActualWord, tActualCol1, tActualRow1, tActualTot1, tActualCol2, tActualRow2, tActualTot2)){	//data word is data record if true is returned
			tNdataRecord++;										//increase data record counter
			_nDataRecords++;									//increase total data record counter
			if(tActualTot1 >= 0)								//add hit if hit info is reasonable (TOT1 >= 0)
				addHit(tActualCol1, tActualRow1, tDbCID, tActualTot1);
			if(tActualTot2 >= 0)								//add hit if hit info is reasonable and set (TOT2 >= 0)
				addHit(tActualCol2, tActualRow2, tDbCID, tActualTot2);
			if(__CLUSTERIZER_DEBUG & _once) std::cout<<" DR COL1/ROW1/TOT1  COL2/ROW2/TOT2 "<<tActualCol1<<"/"<<tActualRow1<<"/"<<tActualTot1<<"  "<<tActualCol2<<"/"<<tActualRow2<<"/"<<tActualTot2<<" rBCID "<<tDbCID<<"\n";
		}
		else if (isTriggerWord(tActualWord)){					//data word is trigger word, is last word of the event data if external trigger is present, cluster data
			_nTriggers++;										//increase the trigger word counter
			int tTriggerNumber = TRIGGER_NUMBER_MACRO2(tActualWord, pSRAMwordRB[++iWord]);//actual trigger number
			if(__CLUSTERIZER_DEBUG & _once) std::cout<<"Clusterizer::clusterRawData: TRIGGER "<<tTriggerNumber<<std::endl;
			if(tTriggerNumber == (int)_nTriggers && tValidTriggerData && tNdataHeader == _NbCID && tLVL1IDisConst){	//sanity check, only cluster good event data
				if(_bCIDfirstHit != -1){						//_bCIDfirstHit == -1 if no data record thus no hit exists in the event window
					if (!clusterize()){							//start the clustering
						if(__CLUSTERIZER_DEBUG) std::cout<<"Clusterizer::clusterRawData: FAILED TO CLUSTER TRIGGERED DATA"<<std::endl;
						return false;
					}
					_nEvents++;
					if(__CLUSTERIZER_DEBUG && _once)
						printEventData();
				}
			}
			else{
				if(__CLUSTERIZER_DEBUG){
					std::cout<<"Clusterizer::clusterRawData: WRONG TRIGGERED EVENT DATA AT "<<iWord<<"\n";
					showHits();
					printEventData();
				}
				clearHitMap();									//clustering is destructive and deletes hits, without successfull clustering they have to be deleted as done here
				_nTriggers = tTriggerNumber;
				_nInvalidEvents++;
			}
			_once = false;
			resetEventVariables();
		}
		else{
			if (!isOtherWord(tActualWord)){						//other for clustering uninteressting data, else data word unknown
				tValidTriggerData = false;
				if(__CLUSTERIZER_DEBUG) std::cout<<"Clusterizer::clusterRawData: UNKNOWN WORD "<<tActualWord<<" AT "<<iWord<<"\n";
			}
		}

		if (tIncompleteEvent){	//tIncompleteEvent is raised if BCID is not increasing by 1, most likely due to incomplete data transmission, so start new event, actual word is data header here
			if(__CLUSTERIZER_DEBUG & _once) std::cout<<"INCOMPLETE EVENT DATA STRUCTURE\n";
			if (tNdataHeader > 2 || _NbCID < 2)
				_nIncompleteEvents++;
			getTimefromDataHeader(tActualWord, tActualLVL1ID, tStartBCID);
			clearHitMap();										//clustering is destructive and deletes hits, without successfull clustering they have to be deleted as done here
			resetEventVariables();
			tNdataHeader = 1;									//tNdataHeader is already 1, because actual word is first data of new event
			tStartBCID = tActualBCID;
			tStartLVL1ID = tActualLVL1ID;
		}

		if(_nInvalidEvents > _maxInvalidEvents)	//maximum invalid events is reached and clustering is aborted, too bad...
			return false;

		if(_nIncompleteEvents > _maxIncompleteEvents)	//maximum incomplete events is reached and clustering is aborted, too bad...
			return false;
	}

	if(__CLUSTERIZER_DEBUG){
		std::cout<<"Clusterizer::clusterRawData summary:"<<std::endl;
		std::cout<<"  _nInvalidEvents "<<_nInvalidEvents<<std::endl;
		std::cout<<"  _nIncompleteEvents "<<_nIncompleteEvents<<std::endl;
		std::cout<<"  _nTriggers "<<_nTriggers<<std::endl;
		std::cout<<"  _nDataRecords "<<_nDataRecords<<std::endl;
		std::cout<<"  _nEvents "<<_nEvents<<std::endl;
		showHistsEntries();
	}
	return true;
}

void Clusterizer::resetRawDataCounters()
{
	if(__CLUSTERIZER_DEBUG) std::cout<<"Clusterizer::resetRawDataCounters\n";
	_nTriggers = 0;
	_nEvents = 0;
	_nDataRecords = 0;
}

void Clusterizer::clearHitMap()
{
	if(__CLUSTERIZER_DEBUG & _once) std::cout<<"Clusterizer::clearHitMap\n";

	if(_nHits != 0){
		for(int iCol = 0; iCol < RAW_DATA_MAX_COLUMN; ++iCol){
			for(int iRow = 0; iRow < RAW_DATA_MAX_ROW; ++iRow){
				for(int iRbCID = 0; iRbCID < __MAXBCID; ++iRbCID){
					if(_hitMap[iCol][iRow][iRbCID] != -1){
						_hitMap[iCol][iRow][iRbCID] = -1;
						_nHits--;
					if(_nHits == 0)
						goto exitLoop;	//the fastest way to exit a nested loop
					}
				}
			}
		}
	}

	exitLoop:
	_minColHitPos = RAW_DATA_MAX_COLUMN-1;
	_maxColHitPos = 0;
	_minRowHitPos = RAW_DATA_MAX_ROW-1;
	_maxRowHitPos = 0;
	_bCIDfirstHit = -1;
	_bCIDlastHit = -1;
	_nHits = 0;
}

void Clusterizer::initHitMap()
{
	if(__CLUSTERIZER_DEBUG & _once) std::cout<<"Clusterizer::initHitMap\n";

	for(int iCol = 0; iCol < RAW_DATA_MAX_COLUMN; ++iCol){
		for(int iRow = 0; iRow < RAW_DATA_MAX_ROW; ++iRow){
			for(int iRbCID = 0; iRbCID < __MAXBCID; ++iRbCID)
				_hitMap[iCol][iRow][iRbCID] = -1;
		}
	}

	_minColHitPos = RAW_DATA_MAX_COLUMN-1;
	_maxColHitPos = 0;
	_minRowHitPos = RAW_DATA_MAX_ROW-1;
	_maxRowHitPos = 0;
	_bCIDfirstHit = -1;
	_bCIDlastHit = -1;
	_nHits = 0;
}

void Clusterizer::initChargeCalibMap()
{
	if(__CLUSTERIZER_DEBUG) std::cout<<"Clusterizer::initChargeCalibMap\n";

	for(int i = 0; i < RAW_DATA_MAX_COLUMN; ++i){
		for(int j = 0; j < RAW_DATA_MAX_ROW; ++j){
			for(int k = 0; k < 14; ++k)
				_chargeMap[i][j][k] = 0;
		}
	}
}

void Clusterizer::clearClusterMaps()
{
	if(__CLUSTERIZER_DEBUG) std::cout<<"Clusterizer::clearClusterMaps\n";
	for (unsigned int i = 0; i < __MAXCLUSTERHITSBINS; ++i)
		_clusterHits[i] = 0;
	for (unsigned int i = 0; i < __MAXTOTBINS; ++i){
		for (unsigned int j = 0; j < __MAXCLUSTERHITSBINS; ++j)
			_clusterTots[i][j] = 0;
	}
	for (unsigned int i = 0; i < __MAXCHARGEBINS; ++i)
		for (unsigned int j = 0; j < __MAXCLUSTERHITSBINS; ++j)
			_clusterCharges[i][j] = 0;
	for (unsigned int i = 0; i < __MAXPOSXBINS; ++i){
		for (unsigned int j = 0; j < __MAXPOSYBINS; ++j)
			_clusterPosition[i][j] = 0;
	}
}

void Clusterizer::addHit(int pCol, int pRow, int pNbCID, int pTot, float pCharge)
{
	if(__CLUSTERIZER_DEBUG && _once){
		std::cout<<"Clusterizer::addHit "<<std::endl;
		std::cout<<"  pCol "<<pCol<<std::endl;
		std::cout<<"  pRow "<<pRow<<std::endl;
		std::cout<<"  pNbCID "<<pNbCID<<std::endl;
		std::cout<<"  pTot "<<pTot<<std::endl;
		std::cout<<"  pCharge "<<pCharge<<std::endl;
	}

	if(_nHits == 0)
		_bCIDfirstHit = pNbCID;

	if(pNbCID > _bCIDlastHit)
		_bCIDlastHit = pNbCID;

	if(pCol > _maxColHitPos)
		_maxColHitPos = pCol;
	if(pCol < _minColHitPos)
		_minColHitPos = pCol;
	if(pRow < _minRowHitPos)
		_minRowHitPos = pRow;
	if(pRow > _maxRowHitPos)
		_maxRowHitPos = pRow;


	_hitMap[pCol][pRow][pNbCID] = pTot;
	if (pCharge >= 0)
		_chargeMap[pCol][pRow][pTot] = pCharge;
	_nHits++;
}

bool Clusterizer::clusterize()
{
	if(__CLUSTERIZER_DEBUG && _once){
		std::cout<<"Clusterizer::clusterize() "<<std::endl;
		std::cout<<"  _nHits "<<_nHits<<std::endl;
		std::cout<<"  _bCIDfirstHit "<<_bCIDfirstHit<<std::endl;
		std::cout<<"  _bCIDlastHit "<<_bCIDlastHit<<std::endl;
		std::cout<<"  _minColHitPos "<<_minColHitPos<<std::endl;
		std::cout<<"  _maxColHitPos "<<_maxColHitPos<<std::endl;
		std::cout<<"  _minRowHitPos "<<_minRowHitPos<<std::endl;
		std::cout<<"  _maxRowHitPos "<<_maxRowHitPos<<std::endl;
	}

	_runTime = 0;
	_abortClustering = false;

	for(int iBCID = _bCIDfirstHit; iBCID <= _bCIDlastHit; ++iBCID){			//loop over the hit array starting from the first hit BCID to the last hit BCID
		for(int iCol = _minColHitPos; iCol <= _maxColHitPos; ++iCol){		//loop over the hit array from the minimum to the maximum column with a hit
			for(int iRow = _minRowHitPos; iRow <= _maxRowHitPos; ++iRow){	//loop over the hit array from the minimum to the maximum row with a hit
				if (_abortClustering)										//abort event is set only in the SearchNextHits(...) method
					return false;
				if(hitExists(iCol,iRow,iBCID)){								//if a hit in iCol,iRow,iBCID exists take this as a first hit of a cluster and do:
					clearActualClusterData();								//  clear the last cluster data
					_actualRelativeClusterBCID = iBCID;						//  set the minimum relative BCID [0:15] for the new cluster
					searchNextHits(iCol, iRow, iBCID);						//  find hits next to the actual one and update the actual cluster values, here the clustering takes place
					if (_actualClusterHits >= (int) _minClusterHits)		//  only add cluster if it has at least _minClusterHits hits
						addClusterToResults();								//  add the actual cluster values to the result histograms
				}
				if (_nHits == 0)											//saves a lot of average run time, the loop is aborted if every hit is in a cluster (_nHits == 0)
					return true;
			}
		}
	}
	if (_nHits == 0)
		return true;
	if(__CLUSTERIZER_DEBUG){
		std::cout<<"Clusterizer::clusterize: NOT ALL HITS CLUSTERED!"<<std::endl;
		showHits();
	}
	return false;
}

void Clusterizer::searchNextHits(int pCol, int pRow, int pBCID)
{
	if(__CLUSTERIZER_DEBUG && _once){
		std::cout<<"Clusterizer::searchNextHits "<<std::endl;
		std::cout<<"  _nHits "<<_nHits<<std::endl;
		std::cout<<"  _abortClustering "<<_abortClustering<<std::endl;
		std::cout<<"  _actualRelativeClusterBCID "<<_actualRelativeClusterBCID<<std::endl;
		std::cout<<"  pBCID "<<pBCID<<std::endl;
		std::cout<<"  _NbCID "<<_NbCID<<std::endl;
		std::cout<<"  _DbCID "<<_DbCID<<std::endl;
		showHits();
	}

	if(_abortClustering){	//if abort flags is set return
		std::cout<<"Clusterizer::searchNextHits ABORT"<<std::endl;
		return;
	}

	_actualClusterHits++;	//increase the total hits for this cluster value

	short int tTot = _hitMap[pCol][pRow][pBCID];

	if(tTot > (short int) _maxClusterHitTot)	//omit cluster with a hit tot higher than _maxClusterHitTot, clustering is not aborted to delete all hits from this cluster from the hit array
		_abortCluster = true;

	if(_actualClusterHits > (int) _maxClusterHits)		//omit cluster if it has more hits than _maxClusterHits, clustering is not aborted to delete all hits from this cluster from the hit array
		_abortCluster = true;

	_actualClusterTot+=tTot;		//add tot of the hit to the cluster tot
	_actualClusterCharge+=_chargeMap[pCol][pRow][tTot];	//add charge of the hit to the cluster tot
	_actualClusterX+=((double) pCol+0.5) * __PIXELSIZEX * _chargeMap[pCol][pRow][tTot];	//add x position of actual cluster weigthed by the charge
	_actualClusterY+=((double) pRow+0.5) * __PIXELSIZEY * _chargeMap[pCol][pRow][tTot];	//add y position of actual cluster weigthed by the charge

	if(__CLUSTERIZER_DEBUG && _once){
		std::cout<<"Clusterizer::searchNextHits"<<std::endl;
		std::cout<<"  pCol "<<pCol<<std::endl;
		std::cout<<"  pRow "<<pRow<<std::endl;
		std::cout<<"  _chargeMap[pCol][pRow][tTot] "<<_chargeMap[pCol][pRow][tTot]<<std::endl;
		std::cout<<"  ((double) pCol+0.5) * __PIXELSIZEX "<<((double) pCol+0.5) * __PIXELSIZEX<<std::endl;
		std::cout<<"  ((double) pRow+0.5) * __PIXELSIZEY "<<((double) pRow+0.5) * __PIXELSIZEY<<std::endl;
		std::cout<<"  _actualClusterX "<<_actualClusterX<<std::endl;
		std::cout<<"  _actualClusterY "<<_actualClusterY<<std::endl;
	}

	if (deleteHit(pCol, pRow, pBCID))	//delete hit and return if no hit is in the array anymore
		return;

	//values set to true to avoid double searches in one direction with different step sizes
	bool tHitUp = false;
	bool tHitUpRight = false;
	bool tHitRight = false;
	bool tHitDownRight = false;
	bool tHitDown = false;
	bool tHitDownLeft = false;
	bool tHitLeft = false;
	bool tHitUpLeft = false;

	//search around the pixel in time and space
	for(int iDbCID = _actualRelativeClusterBCID; iDbCID <= _actualRelativeClusterBCID +_DbCID && iDbCID <= _bCIDlastHit; ++iDbCID){	//loop over the BCID window width starting from the actual cluster BCID
		for(int iDx = 1; iDx <= (int) _dx; ++iDx){									//loop over the the x range
			for(int iDy = 1; iDy <= (int) _dy; ++iDy){								//loop over the the y range
				_runTime++;
				if(hitExists(pCol,pRow+iDy,iDbCID) && !tHitUp){					//search up
					tHitUp = true;
					searchNextHits(pCol, pRow+iDy, iDbCID);
				}
				if(hitExists(pCol+iDx,pRow+iDy,iDbCID) && !tHitUpRight){		//search up, right
					tHitUpRight = true;
					searchNextHits(pCol+iDx, pRow+iDy, iDbCID);
				}
				if(hitExists(pCol+iDx, pRow,iDbCID) && !tHitRight){				//search right
					tHitRight = true;
					searchNextHits(pCol+iDx, pRow, iDbCID);
				}
				if(hitExists(pCol+iDx, pRow-iDy,iDbCID) && !tHitDownRight){		//search down, right
					tHitDownRight = true;
					searchNextHits(pCol+iDx, pRow-iDy, iDbCID);
				}
				if(hitExists(pCol, pRow-iDy,iDbCID) && !tHitDown){				//search down
					tHitDown = true;
					searchNextHits(pCol, pRow-iDy, iDbCID);
				}
				if(hitExists(pCol-iDx, pRow-iDy,iDbCID) && !tHitDownLeft){		//search down, left
					tHitDownLeft = true;
					searchNextHits(pCol-iDx, pRow-iDy, iDbCID);
				}
				if(hitExists(pCol-iDx, pRow,iDbCID) && !tHitLeft){				//search left
					tHitLeft = true;
					searchNextHits(pCol-iDx, pRow, iDbCID);
				}
				if(hitExists(pCol-iDx, pRow+iDy,iDbCID) && !tHitUpLeft){		//search up, left
					tHitUpLeft = true;
					searchNextHits(pCol-iDx, pRow+iDy, iDbCID);
				}
			}
		}
	}
}

bool Clusterizer::hitExists(int pCol, int pRow, int pBCID)
{
	if(pCol>-1 && pCol < RAW_DATA_MAX_COLUMN && pRow > -1 && pRow < RAW_DATA_MAX_ROW && pBCID > -1 && pBCID < __MAXBCID)
		if(_hitMap[pCol][pRow][pBCID] != -1)
			return true;
	return false;
}

bool Clusterizer::deleteHit(int pCol, int pRow, int pBCID)
{
	_hitMap[pCol][pRow][pBCID] = -1;
	_nHits--;
	if(_nHits == 0){
		_minColHitPos = RAW_DATA_MAX_COLUMN-1;
		_maxColHitPos = 0;
		_minRowHitPos = RAW_DATA_MAX_ROW-1;
		_maxRowHitPos = 0;
		_bCIDfirstHit = -1;
		_bCIDlastHit = -1;
		return true;
	}
	return false;
}

void Clusterizer::clearActualClusterData()
{
	_actualClusterTot = 0;
	_actualClusterHits = 0;
	_actualClusterCharge = 0;
	_actualRelativeClusterBCID = 0;
	_actualClusterX = 0;
	_actualClusterY = 0;
	_abortCluster = false;					//reset abort flag for the new cluster
}


void Clusterizer::setXclusterDistance(unsigned int pDx)
{
	if(__CLUSTERIZER_DEBUG)
		std::cout<<"Clusterizer::setXclusterDistance: "<<pDx<<"\n";
	if (pDx > 1 && pDx < RAW_DATA_MAX_COLUMN-1)
		_dx = pDx;
}

void Clusterizer::setYclusterDistance(unsigned int pDy)
{
	if(__CLUSTERIZER_DEBUG)
		std::cout<<"Clusterizer::setYclusterDistance: "<<pDy<<"\n";
	if (pDy > 1 && pDy < RAW_DATA_MAX_ROW-1)
		_dy = pDy;
}

void Clusterizer::setBCIDclusterDistance(unsigned int pDbCID)
{
	if(__CLUSTERIZER_DEBUG)
		std::cout<<"Clusterizer::setBCIDclusterDistance: "<<pDbCID<<"\n";
	if (pDbCID < __MAXBCID-1)
		_DbCID = pDbCID;
}

unsigned int Clusterizer::getRunTime()
{
	return _runTime;
}

void Clusterizer::setNbCIDs(int NbCIDs)
{
	_NbCID = NbCIDs;
}

void Clusterizer::showHits()
{
	std::cout<<"ShowHits"<<std::endl;
	if(_nHits < 100){
		for(int i = 0; i < RAW_DATA_MAX_COLUMN; ++i){
			for(int j = 0; j < RAW_DATA_MAX_ROW; ++j){
				for(int k = 0; k < __MAXBCID; ++k){
					if (_hitMap[i][j][k] != -1)
						std::cout<<"x/y/BCID/Tot = "<<i<<"/"<<j<<"/"<<k<<"/"<<_hitMap[i][j][k]<<std::endl;
				}
			}
		}
	}
	else
		std::cout<<"TOO MANY HITS =  "<<_nHits<<" TO SHOW!"<<std::endl;
}

void Clusterizer::showHistsEntries()
{
	std::cout<<"Number of hits per cluster\n";
	for (unsigned int i = 0; i < __MAXCLUSTERHITSBINS; ++i){
		std::cout<<_clusterHits[i]<<" ";
	}
	std::cout<<"\n Cluster Charge\n";
	for (unsigned int i = 0; i < __MAXTOTBINS; ++i){
		std::cout<<_clusterCharges[i][0]<<" ";
		if(i%100 == 0) std::cout<<std::endl;
	}
	std::cout<<"\n Cluster Tot\n";
	for (unsigned int i = 0; i < __MAXTOTBINS; ++i){
		std::cout<<_clusterTots[i][0]<<" ";
	}
	std::cout<<std::endl;
}

void Clusterizer::setFEI4B(bool pIsFEI4B)
{
	if(__CLUSTERIZER_DEBUG)
		std::cout<<"Clusterizer::setFEI4B "<<pIsFEI4B<<"\n";
	_fEI4B = pIsFEI4B;
}

unsigned long Clusterizer::getClusterSizeOccurrence(int pClusterSize)
{
	if (pClusterSize >= 0 && pClusterSize < __MAXCLUSTERHITSBINS)
		return _clusterHits[pClusterSize];
	return 0;
}

unsigned long Clusterizer::getClusterTotOccurrence(int pClusterTot)
{
	if (pClusterTot >= 0 && pClusterTot < __MAXTOTBINS)
		return _clusterTots[pClusterTot][0];
	return 0;
}

unsigned long Clusterizer::getClusterChargeAndSizeOccurrence(int pClusterCharge, int pClusterSize)
{
	if (pClusterCharge >= 0 && pClusterCharge < __MAXCHARGEBINS && pClusterSize >= 0 && pClusterSize < __MAXCLUSTERHITSBINS)
		return _clusterCharges[pClusterCharge][pClusterSize];
	return 0;
}

unsigned long Clusterizer::getClusterPositionOccurrence(int pClusterPosX, int pClusterPosY)
{
	if (pClusterPosX >= 0 && pClusterPosX < __MAXPOSXBINS && pClusterPosY > 0 && pClusterPosY < __MAXPOSYBINS)
		return _clusterPosition[pClusterPosX][pClusterPosY];
	return 0;
}

unsigned long Clusterizer::getClusterTotAndSizeOccurrence(int pClusterTot, int pClusterSize)
{
	if (pClusterTot >= 0 && pClusterTot < __MAXTOTBINS && pClusterSize >= 0 && pClusterSize < __MAXCLUSTERHITSBINS)
		return _clusterTots[pClusterTot][pClusterSize];
	return 0;
}

void Clusterizer::setPixelChargeCalibration(unsigned int pCol, unsigned int pRow, unsigned int pTot, float pCharge)
{
	if(pCol < RAW_DATA_MAX_COLUMN && pRow < RAW_DATA_MAX_ROW && pTot < 14)
		_chargeMap[pCol][pRow][pTot] = pCharge;
}

void Clusterizer::setMinClusterHits(unsigned int pMinNclusterHits)
{
	if(__CLUSTERIZER_DEBUG)
		std::cout<<"Clusterizer::setMinClusterHits "<<pMinNclusterHits<<"\n";
	_minClusterHits = pMinNclusterHits;
}

void Clusterizer::setMaxClusterHits(unsigned int pMaxNclusterHits)
{
	if(__CLUSTERIZER_DEBUG)
			std::cout<<"Clusterizer::setMaxClusterHits "<<pMaxNclusterHits<<"\n";
	_maxClusterHits = pMaxNclusterHits;
}

void Clusterizer::setMaxWrongEvents(unsigned int pMaxWrongEvents)
{
	if(__CLUSTERIZER_DEBUG)
			std::cout<<"Clusterizer::setMaxWrongEvents "<<pMaxWrongEvents<<"\n";
	_maxInvalidEvents = (unsigned long) pMaxWrongEvents;
}

void Clusterizer::setMaxIncompleteEvents(unsigned int pMaxIncompleteEvents)
{
	if(__CLUSTERIZER_DEBUG)
			std::cout<<"Clusterizer::setMaxIncompleteEvents "<<pMaxIncompleteEvents<<"\n";
	_maxIncompleteEvents = (unsigned long) pMaxIncompleteEvents;
}

void Clusterizer::setMaxClusterHitTot(unsigned int pMaxClusterHitTot)
{
	if(__CLUSTERIZER_DEBUG)
		std::cout<<"Clusterizer::setMaxClusterHitTot "<<pMaxClusterHitTot<<"\n";
	_maxClusterHitTot = pMaxClusterHitTot;
}

void Clusterizer::addClusterToResults()
{
	if(!_abortCluster && !_abortClustering){
		//histogramming of the results
		if(_actualClusterHits<__MAXCLUSTERHITSBINS)
			_clusterHits[_actualClusterHits]++;
		if(_actualClusterTot<__MAXTOTBINS && _actualClusterHits<__MAXCLUSTERHITSBINS){
			_clusterTots[_actualClusterTot][0]++;	//cluster size = 0 contains all cluster sizes
			_clusterTots[_actualClusterTot][_actualClusterHits]++;
		}
		if((int) _actualClusterCharge<__MAXCHARGEBINS && _actualClusterHits<__MAXCLUSTERHITSBINS){
			_clusterCharges[(int) _actualClusterCharge][0]++;
			_clusterCharges[(int) _actualClusterCharge][_actualClusterHits]++;
		}
		if(_actualClusterCharge > 0){	//avoid division by zero
			_actualClusterX/=_actualClusterCharge;
			_actualClusterY/=_actualClusterCharge;
			int tActualClusterXbin = (int) (_actualClusterX/(__PIXELSIZEX*RAW_DATA_MAX_COLUMN) * __MAXPOSXBINS);
			int tActualClusterYbin = (int) (_actualClusterY/(__PIXELSIZEY*RAW_DATA_MAX_ROW) * __MAXPOSYBINS);
			if(tActualClusterXbin < __MAXPOSXBINS && tActualClusterYbin < __MAXPOSYBINS)
				_clusterPosition[tActualClusterXbin][tActualClusterYbin]++;
		}
	}
}

bool Clusterizer::getTimefromDataHeader(unsigned int& pSRAMWORD, unsigned int& pLVL1ID, unsigned int& pBCID)
{
	if (DATA_HEADER_MACRO(pSRAMWORD)){
		if (_fEI4B){
			pLVL1ID = DATA_HEADER_LV1ID_MACRO_FEI4B(pSRAMWORD);
			pBCID = DATA_HEADER_BCID_MACRO_FEI4B(pSRAMWORD);
		}
		else{
			pLVL1ID = DATA_HEADER_LV1ID_MACRO(pSRAMWORD);
			pBCID = DATA_HEADER_BCID_MACRO(pSRAMWORD);
		}
		return true;
	}
	return false;
}

bool Clusterizer::getHitsfromDataRecord(unsigned int& pSRAMWORD, int& pColHit1, int& pRowHit1, int& pTotHit1, int& pColHit2, int& pRowHit2, int& pTotHit2)
{
	if (DATA_RECORD_MACRO(pSRAMWORD)){	//SRAM word is data record
		//check if the hit values are reasonable
		if ((DATA_RECORD_TOT1_MACRO(pSRAMWORD) == 0xF) || 
		    (DATA_RECORD_COLUMN1_MACRO(pSRAMWORD) < RAW_DATA_MIN_COLUMN) || 
		    (DATA_RECORD_COLUMN1_MACRO(pSRAMWORD) > RAW_DATA_MAX_COLUMN) || 
		    (DATA_RECORD_ROW1_MACRO(pSRAMWORD) < RAW_DATA_MIN_ROW) || 
		    ((DATA_RECORD_ROW1_MACRO(pSRAMWORD) > RAW_DATA_MAX_ROW)
		     && (DATA_RECORD_TOT2_MACRO(pSRAMWORD) != 0xF) && 
		     ((DATA_RECORD_COLUMN2_MACRO(pSRAMWORD) < RAW_DATA_MIN_COLUMN) || 
		      (DATA_RECORD_COLUMN2_MACRO(pSRAMWORD) > RAW_DATA_MAX_COLUMN) || 
		      (DATA_RECORD_ROW2_MACRO(pSRAMWORD) < RAW_DATA_MIN_ROW) || 
		      (DATA_RECORD_ROW2_MACRO(pSRAMWORD) > RAW_DATA_MAX_ROW)))){
			if (__CLUSTERIZER_DEBUG)
				std::cout<<"Clusterizer::getHitsfromDataRecord: ERROR data record values out of bounds"<<std::endl;
			return false;
		}

		//set first hit values
		if (DATA_RECORD_TOT1_MACRO(pSRAMWORD) <= __MAXTOTUSED){	//ommit late/small hit and no hit TOT values for the TOT(1) hit
			pColHit1 = DATA_RECORD_COLUMN1_MACRO(pSRAMWORD);
			pRowHit1 = DATA_RECORD_ROW1_MACRO(pSRAMWORD);
			pTotHit1 = DATA_RECORD_TOT1_MACRO(pSRAMWORD);
		}

		//set second hit values
		if (DATA_RECORD_TOT2_MACRO(pSRAMWORD) <= __MAXTOTUSED){	//ommit late/small hit (14) and no hit (15) tot values for the TOT(2) hit
			pColHit2 = DATA_RECORD_COLUMN2_MACRO(pSRAMWORD);
			pRowHit2 = DATA_RECORD_ROW2_MACRO(pSRAMWORD);
			pTotHit2 = DATA_RECORD_TOT2_MACRO(pSRAMWORD);
		}
		return true;
	}
	return false;
}

bool Clusterizer::isTriggerWord(unsigned int& pSRAMWORD)
{
	if (TRIGGER_WORD_MACRO(pSRAMWORD))	//data word is trigger word
		return true;
	return false;
}

bool Clusterizer::isOtherWord(unsigned int& pSRAMWORD)
{
	if (EMPTY_RECORD_MACRO(pSRAMWORD) || ADDRESS_RECORD_MACRO(pSRAMWORD) || VALUE_RECORD_MACRO(pSRAMWORD) || SERVICE_RECORD_MACRO(pSRAMWORD))
		return true;
	return false;
}

void Clusterizer::resetEventVariables()
{
	//reset event variables
	tNdataHeader = 0;
	tNdataRecord = 0;
	tDbCID = 0;
	tValidTriggerData = true;
	tIncompleteEvent = false;
	tLVL1IDisConst = true;
}

void Clusterizer::printEventData()
{
	std::cout<<"Clusterizer::printEventData() \n";
	std::cout<<"  Trigger "<<_nTriggers<<"\n";
	std::cout<<"  Event "<<_nEvents<<"\n";
	std::cout<<"    BCID window "<<tDbCID<<", expected "<<_NbCID-1<<"\n";
	std::cout<<"    tStartBCID "<<tStartBCID<<"\n";
	std::cout<<"    tNdataHeader "<<tNdataHeader<<"\n";
	std::cout<<"    tNdataRecord "<<tNdataRecord<<"\n";
	std::cout<<"    _bCIDfirstHit "<<_bCIDfirstHit<<"\n";
	std::cout<<"    _bCIDlastHit "<<_bCIDlastHit<<"\n";
	std::cout<<"    tValidTriggerData "<<tValidTriggerData<<std::endl;
	std::cout<<"    tIncompleteEvent "<<tIncompleteEvent<<std::endl;
}

