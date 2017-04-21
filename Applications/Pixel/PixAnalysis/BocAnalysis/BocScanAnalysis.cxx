/***************************************************************************
                          BocScanAnalysis -  description
                             -------------------
    begin                : Sun, 7 Dec 2005
    author               : Iris Rottlaender
    email                : rottlaen@physik.uni-bonn.de
*************************************************************************/

#include "BocScanAnalysis.h"

void BocScanAnalysisResults::DumpResults()
{
  if(seenError == false)
    {
      cout<<"+++ NO ERRORS FOUND - YOU CAN CHOOSE ANY PARAMETER SETTING. +++"<<endl;
      return;
    }
  
  cout<<endl;
  cout<<"Maximal distance to errors was "<<maxdist<<"."<<endl<<endl;
  
  if(bestX.size()==0)
    {
      cout<<"+++ FOUND NO GOOD PARAMETER REGION! +++"<<endl;
      return;
    }
  
  cout<<"+++ RECOMMENDED PARAMETER VALUES +++"<<endl<<endl;
  for(unsigned int i=0;i<bestX.size();i++)
    {
      cout<<"X-Value : "<<bestX.at(i);
      cout<<",  Y-Value : "<<bestY.at(i);
      cout<<",  Z-Value : "<<bestZ.at(i)<<endl;
    }
  
  cout<<endl;
  return;
}

BocScanAnalysisResults::BocScanAnalysisResults()
{
  dim = 0;
  maxdist = 0;
  seenError = true;
}

BocScanAnalysisResults::~BocScanAnalysisResults()
{
}


BocScanAnalysis::DistanceMatrix::DistanceMatrix(int X, int Y, int Z)
{
  XBins = X;
  YBins = Y;
  ZBins = Z;
  
  // dynamic multidimensional arrray
  // there is no other way to declare this than as "array of arrays"
  
  matrix = new short**[X];
 
  for(int i = 0; i<XBins ; i++)
    {
      matrix[i] = new short*[Y];
      
      for(int j = 0; j<YBins; j++)
	matrix[i][j] = new short[Z];
    }
  
}

BocScanAnalysis::DistanceMatrix::~DistanceMatrix()
{
  if(!(matrix==0))
    {
      for(int i = 0; i<XBins ; i++)
	{
	  if(!(matrix[i]==0))
	    {
	      for(int j = 0; j<YBins; j++)
		{
		  if(!(matrix[i][j]==0))
		    delete[] matrix[i][j];
		}
	      delete[] matrix[i];
	    }
	}
      delete[] matrix;
    }
}


void BocScanAnalysis::DistanceMatrix::DumpMatrix()
{
  cout<<"Distance Matrix:"<<endl;
  for(int zz =0; zz<ZBins; zz++)
    {
      for(int yy =0; yy<YBins; yy++)
	{
	  for(int xx =0; xx<XBins; xx++)
	    {
	      if(matrix[xx][yy][zz]<10&&matrix[xx][yy][zz]>-1)
		cout<<" "<<matrix[xx][yy][zz]<<" ";
	      else
		cout<<matrix[xx][yy][zz]<<" ";
	    }
	  cout<<endl;
	}
    }
}

void BocScanAnalysis::DistanceMatrix::Initialize()
{ 

  for(int zz = 0; zz<ZBins; zz++)
    {
      for(int yy =0; yy<YBins; yy++)
	{
	  for(int xx =0; xx<XBins; xx++)
	    {	      
	      matrix[xx][yy][zz] = -1;
	    }
	}
    }
}

void BocScanAnalysis::DistanceMatrix::FillError(int x, int y, int z)
{
  matrix[x][y][z] = 0;
}



BocScanAnalysisResults BocScanAnalysis::DistanceMatrix::ComputeDistances(ScanSpecs X, ScanSpecs Y, ScanSpecs Z)
{ 
  bool ready = false;
  bool written = false;
  short distance = 0; 

  if(!(X.pacman)&&(XBins>1))
    {
      for(int zz = 0; zz<Z.Bins; zz++)
	{
	  for(int yy =0; yy<Y.Bins; yy++)
	    {
	      if(matrix[0][yy][zz]<0)
		matrix[0][yy][zz]=1;
	      
	      if(matrix[X.Bins-1][yy][zz]<0)
		matrix[X.Bins-1][yy][zz]=1;
	    }
	}
    }
  
  if(!(Y.pacman)&&(YBins>1))
    {
      for(int zz = 0; zz<Z.Bins; zz++)
	{
	  for(int xx =0; xx<X.Bins; xx++)
	    {
	      if(matrix[xx][0][zz]<0)
		matrix[xx][0][zz]=1;

	      if(matrix[xx][Y.Bins-1][zz]<0)
		matrix[xx][Y.Bins-1][zz]=1;
		  
	    }
	}
    }
  
  if(!(Z.pacman)&&(ZBins>1))
    {
      for(int yy = 0; yy<Y.Bins; yy++)
	{
	  for(int xx =0; xx<X.Bins; xx++)
	    {
	      if(matrix[xx][yy][0]<0)
		matrix[xx][yy][0]=1;

	      if(matrix[xx][yy][Z.Bins-1]<0)
		matrix[xx][yy][Z.Bins-1]=1;
	      
	    }
	}
    }
  
  while(!ready)
    {
      ready = true;
      written = false;
      distance++;
      
      for(int zz = 0; zz<Z.Bins; zz++)
	{
	  for(int yy =0; yy<Y.Bins; yy++)
	    {
	      for(int xx =0; xx<X.Bins; xx++)
		{
		  if(matrix[xx][yy][zz]<0)
		    ready = false;
		  
		  if(matrix[xx][yy][zz]==(distance-1))
		    {
		      if(Z.Bins>1)
			{
			  if(Z.pacman)
			    {
			      if(matrix[xx][yy][(zz+1)%Z.Bins]<0)
				{
				  matrix[xx][yy][(zz+1)%Z.Bins] = distance;
				  written = true;
				}
			      
			      int index;
			      if ((zz-1)<0)
				index = Z.Bins-1;
			      else
				index = zz-1;
			      
			      if(matrix[xx][yy][index]<0)
				{
				  matrix[xx][yy][index] = distance;
				  written = true;
				}
			    }
			  else
			    {
			      if(((zz+1)<Z.Bins)&&(matrix[xx][zz][zz+1]<0))
				{
				  matrix[xx][yy][zz+1] = distance;
				  written = true;
				}
			      
			      if(((zz-1)>=0)&&(matrix[xx][yy][zz-1]<0))
				{
				  matrix[xx][yy][zz-1] = distance;
				  written = true;
				}		
			    }  
			}

		      if(Y.Bins > 1)
			{
			  if(Y.pacman)
			    {
			      if(matrix[xx][(yy+1)%Y.Bins][zz]<0)
				{
				  matrix[xx][(yy+1)%Y.Bins][zz] = distance;
				  written = true;
				}
			      
			      int index;
			      if ((yy-1)<0)
				index = Y.Bins-1;
			      else
				index = yy-1;
			      
			      if(matrix[xx][index][zz]<0)
				{
				  matrix[xx][index][zz] = distance;
				  written = true;
				}
			    }
			  else
			    {
			      if(((yy+1)<Y.Bins)&&(matrix[xx][yy+1][zz]<0))
				{
				  matrix[xx][yy+1][zz] = distance;
				  written = true;
				}
			      
			      if(((yy-1)>=0)&&(matrix[xx][yy-1][zz]<0))
				{
				  matrix[xx][yy-1][zz] = distance;
				  written = true;
				}		
			    }  
			}
		  
		      if(X.Bins > 1)
			{
			  if(X.pacman){
			    
			    if(matrix[(xx+1)%X.Bins][yy][zz]<0)
			      {
				matrix[(xx+1)%X.Bins][yy][zz] = distance;
				written=true;
			      }
			    
			    int index;
			    if ((xx-1)<0)
			      index = X.Bins-1;
			    else
			      index = xx-1;
			    
			    if(matrix[index][yy][zz]<0)
			      {
				matrix[index][yy][zz]=distance;
				written = true;
			      }
			  }
			  else
			    {
			      if(((xx+1)<X.Bins)&&(matrix[xx+1][yy][zz]<0))
				{
				  matrix[xx+1][yy][zz] = distance;
				  written = true;
				}
			      
			      if(((xx-1)>=0)&&(matrix[xx-1][yy][zz]<0))
				{
				  matrix[xx-1][yy][zz] = distance;
				  written = true;
				}
			    }
			}
		    }
		}
	    }
	}     
    }
  
  int maxdistance;
  BocScanAnalysisResults results;
 
  if(written)
    maxdistance = distance;
  else
    maxdistance = distance-1;

  results.maxdist = maxdistance;
  
  if(maxdistance >0)
    {  
      for(int zz = 0; zz<Z.Bins; zz++)
	{
	  for(int yy =0; yy<Y.Bins; yy++) 
	    {
	      for(int xx =0; xx<X.Bins; xx++)
		{
		  if(matrix[xx][yy][zz]==maxdistance)
		    { 
		      results.bestX.push_back((int)(X.min + xx*X.BinWidth));
		      results.bestY.push_back((int)(Y.min + yy*Y.BinWidth));
		      results.bestZ.push_back((int)(Z.min + zz*Z.BinWidth));
		      results.modmaxdistX.push_back(results.maxdist);
		      results.modmaxdistY.push_back(results.maxdist);
		      results.modmaxdistZ.push_back(results.maxdist);
		    }
		}
	    }
	}
    }

  return results;
}

BocScanAnalysisResults BocScanAnalysis::FindBestParameter()
{ 
  
  if(histovec.size()<1)
    {
      cout<<"ERROR in BocScanAnalysis::FindBestParameter - Empty histogramm vector"<<endl;
    }

  if(ndim==1)
    {
      cout<<"Performing 1-dimensional BocScanAnalysis (using y-dimension for technical reasons)"<<endl<<endl;
    }
  else if(ndim==2)
    {
      cout<<"Performing 2-dimensional BocScanAnalysis"<<endl<<endl;
    }
  else if(ndim==3)
    {
      cout<<"Performing 3-dimensional BocScanAnalysis"<<endl<<endl;
    }


 cout<<"Number of Events : "<<nevents<<endl<<endl;

 if(nevents==0)
   {
     cout<<"ERROR in BocScanAnalysis::FindBestParameter - No events in histogram!"<<endl;
   }
 
  cout<<"Min X Value      : "<<Dim[0].min<<endl;
  cout<<"Max X Value      : "<<Dim[0].max<<endl;
  cout<<"Number of X Bins : "<<Dim[0].Bins<<endl;
  cout<<"Bin Width X      : "<<Dim[0].BinWidth<<endl<<endl;

  cout<<"Min Y Value      : "<<Dim[1].min<<endl;
  cout<<"Max Y Value      : "<<Dim[1].max<<endl;
  cout<<"Number of Y Bins : "<<Dim[1].Bins<<endl;
  cout<<"Bin Width Y      : "<<Dim[1].BinWidth<<endl<<endl;

  cout<<"Min Z Value      : "<<Dim[2].min<<endl;
  cout<<"Max Z Value      : "<<Dim[2].max<<endl;
  cout<<"Number of Z Bins : "<<Dim[2].Bins<<endl;
  cout<<"Bin Width Z      : "<<Dim[2].BinWidth<<endl<<endl;

  if(Dim[0].Bins==0&&Dim[1].Bins==0&&Dim[2].Bins==0)
    {
      cout<<"ERROR in BocScanAnalysis::FindBestParameter - Can't analyse zero-dimensional histograms"<<endl;
    }
  
  DistanceMatrix matrix = DistanceMatrix(Dim[0].Bins, Dim[1].Bins, Dim[2].Bins);
  matrix.Initialize();
  
  /*
    cout<<"Matrix after Initialization"<<endl;
    matrix.DumpMatrix();
  */
  
  bool SeenError = false;
    
  for(int zz = 0; zz<Dim[2].Bins; zz++)
    {
      for(int yy = 0; yy<Dim[1].Bins; yy++)
	{
	  for(int xx = 0; xx<Dim[0].Bins; xx++)
	    {  
	      if(((histovec.at(zz)))(xx,yy)!=0)
		{
		  SeenError = true;
		  matrix.FillError(xx,yy,zz);
		}
	    }
	}
    }
  
  //cout<<"After Filling Errors"<<endl;
  //matrix.DumpMatrix();
  
  
  BocScanAnalysisResults results;

  if(SeenError)
    {
      results = matrix.ComputeDistances(Dim[0], Dim[1], Dim[2]);
      // cout<<"After Computing distances"<<endl;
      // matrix.DumpMatrix();
    }
  
  results.seenError = SeenError;
  results.dim = ndim;
  results.param[0] = Dim[0].ParamLoop;
  results.param[1] = Dim[1].ParamLoop;
  results.param[2] = Dim[2].ParamLoop;
  results.inlink = inlink;
  for(int i=0; i<4; i++)
    results.outlink[i]=outlink[i];

  // for further use, use x-dim for 1dim scan in results.
  if (ndim == 1)
    {
      vector<int> tmpvec;
      tmpvec = results.bestX;
      results.bestX = results.bestY;
      results.bestY = tmpvec;
    } 

  // if the Rx-Threshold is one of the scanned parameters: look if the upmost scanned value for the threshold
  // shows only errors or also good regions. if only errors, the upper threshold is presumably visible in the scan, se    t seeUpperThreshold accordingly.

  results.seeUpperThreshold = false;
  
  for(int i =0; i<3; i++)
    {
      if (results.param[i]=="BOC_RX_THR")
	{
	  results.seeUpperThreshold = true;
	
	  int starthelp[3] = {0,0,0};

	  starthelp[i] = Dim[i].Bins-1;

	  
	  for(int zz = starthelp[2]; zz<Dim[2].Bins; zz++)
	    {
	      for(int yy =starthelp[1]; yy<Dim[1].Bins; yy++) 
		{
		  for(int xx =starthelp[0]; xx<Dim[0].Bins; xx++)
		    {
		      if(((histovec.at(zz)))(xx,yy)==0)
			results.seeUpperThreshold = false;
		    }
		}
	    }
	}
    }


  // if dim = 2 the distance to parameter which is closer to errors might dominate results, to avoid this:
  if((ndim==2))
    {
      // refine Algorithm not independent on order of execution, very crude way for "optimization"
      if(Dim[0].Bins>Dim[1].Bins)
	{
      	  RefineResults(&(results.bestX), &(results.modmaxdistX), &(results.bestY), &(results.modmaxdistY), Dim[0] );
	  RefineResults(&(results.bestY), &(results.modmaxdistY), &(results.bestX), &(results.modmaxdistX), Dim[1] );
	}
      else
	{
	  RefineResults(&(results.bestY), &(results.modmaxdistY), &(results.bestX), &(results.modmaxdistX), Dim[1] );
	  RefineResults(&(results.bestX), &(results.modmaxdistX), &(results.bestY), &(results.modmaxdistY), Dim[0] );
	}
    }

  // shiftResults(&results);

  return results;
}

int BocScanAnalysis::InitializeAnalysis(PixDBData DataObj)
{
  nevents = DataObj.getNevents();

  Config &scanconf = DataObj.getScanConfig();
  
  ConfList & outList0 = (ConfList&)scanconf["loops"]["paramLoop_0"];
  Dim[0].ParamLoop = outList0.sValue();
  cout<<"ParamLoop0 "<<Dim[0].ParamLoop<<endl;
  
  outList0 = (ConfList&)scanconf["loops"]["paramLoop_1"];
  Dim[1].ParamLoop = outList0.sValue();
  cout<<"ParamLoop1 "<<Dim[1].ParamLoop<<endl;

  outList0 = (ConfList&)scanconf["loops"]["paramLoop_2"];
  Dim[2].ParamLoop = outList0.sValue();
  cout<<"ParamLoop2 "<<Dim[2].ParamLoop<<endl;

  string outSpeed;
  ConfList outString = (ConfList&)scanconf["mcc"]["mccBandwidth"]; 
  outSpeed = outString.sValue();
  cout<<"OutSpeed "<<outSpeed<<endl;

  ConfBool & outBool0 = (ConfBool&)scanconf["loops"]["activeLoop_0"];
  Dim[0].LoopUsed = outBool0.value();
  cout<<"LoopUsed0 "<<Dim[0].LoopUsed<<endl;

  ConfBool & outBool1 = (ConfBool&)scanconf["loops"]["activeLoop_1"];
  Dim[1].LoopUsed = outBool1.value();
  cout<<"LoopUsed1 "<<Dim[1].LoopUsed<<endl;
  
  ConfBool & outBool2 = (ConfBool&)scanconf["loops"]["activeLoop_2"];
  Dim[2].LoopUsed = outBool2.value();
  cout<<"LoopUsed2 "<<Dim[2].LoopUsed<<endl;
  
  Config &modconf = DataObj.getModConfig();

  ConfInt & outInt1 = (ConfInt&)modconf["general"]["BocInputLink"];
  inlink = outInt1.getValue();
 
  outInt1 = (ConfInt&)modconf["general"]["BocOutputLink1"];
  outlink[0] = outInt1.getValue();

  outInt1 = (ConfInt&)modconf["general"]["BocOutputLink2"];
  outlink[1] = outInt1.getValue();

  int fmtOff1[8] = { 2, 6, 38, 42, 50, 54, 86, 90 };
  int fmtOff2[8] = { 2, 4, 6, 8, 86, 88, 90, 92 };
  // Compute BOC links in case the record is not found in DB (assuming standard BOC mapping)
  int il, ol[4];

  il     = ((ConfInt&)(modconf)["general"]["InputLink"]).getValue();
  ol[0]  = ((ConfInt&)(modconf)["general"]["OutputLink1"]).getValue();
  ol[1]  = ((ConfInt&)(modconf)["general"]["OutputLink2"]).getValue();
  ol[2]  = ((ConfInt&)(modconf)["general"]["OutputLink3"]).getValue();
  ol[3]  = ((ConfInt&)(modconf)["general"]["OutputLink4"]).getValue();
  
  if (inlink < 0) {
    inlink = il;
  }
  
  //  PixScan::MccBandwidth outSpeed = scn->getMccBandwidth();
  
  if (outlink[0] < 0) {
    if (outSpeed == "SINGLE_40" || outSpeed == "DOUBLE_40") {
      outlink[0] = fmtOff1[ol[0]/16] + ol[0]%16;
    } else {
      outlink[0] = fmtOff2[ol[0]/16] + (ol[0]%16)/2;
    }
  }
  if (outlink[1] < 0) {
    if (outSpeed == "SINGLE_40" || outSpeed == "DOUBLE_40") {
      outlink[1] = fmtOff1[ol[1]/16] + ol[1]%16;
    } else {
      outlink[1] = fmtOff2[ol[2]/16] + (ol[2]%16)/2;
    }
  }
  if (outSpeed == "SINGLE_40" || outSpeed == "SINGLE_80") {
    outlink[1] = -1;
  }
  
  cout<<"Boc Inlink "<<inlink;
  cout<<" Boc Outlink1 "<<outlink[0];
  cout<<" Boc Outlink2 "<<outlink[1]<<endl;
  
  // otherwise crash, because only 2 BocLinks in PixLib
  /*
    outInt1 = (ConfInt&)modconf["general"]["BocOutputLink3"];
    outlink[2] = outInt1.getValue();
    cout<<"Boc Outlink3 "<<outlink[2];

    outInt1 = (ConfInt&)modconf["general"]["BocOutputLink4"];
    outlink[3] = outInt1.getValue();
    cout<<"Boc Outlink4 "<<outlink[3];
  */
  
  //set unused outlinks to -1
  // later: automatic derivision from links 1 & 2 possible?
  for(int i=2; i<4;i++)
    outlink[i]= -1;
  
  for(int i =0; i<3; i++)
    {
      if((Dim[i].ParamLoop=="BOC_RX_DELAY")||(Dim[i].ParamLoop=="BOC_BPH")||(Dim[i].ParamLoop=="BOC_BPMPH")||(Dim[i].ParamLoop=="BOC_TX_BPM")||(Dim[i].ParamLoop=="BOC_VPH0"))
	{
	  Dim[i].pacman=true;
	  Dim[i].minpos = 0;
	  Dim[i].maxpos = 25;
	}
      else
	Dim[i].pacman=false;   
    }
  
  if(Dim[2].LoopUsed)
    ndim = 3;
  else if(Dim[1].LoopUsed)
    ndim = 2;
  else if(Dim[0].LoopUsed)
    ndim = 1;
  else
    {
      cout<<"ERROR - Scan contains no Loop"<<endl;
      return -1;
    } 
  
  cout<<"Dim "<<ndim<<endl;

  PixScan::HistogramType type;
 
  if(ndim==1)
    type = PixScan::RAW_DATA_DIFF_1;
  else if ((ndim==2)||(ndim==3))
    type = PixScan::RAW_DATA_DIFF_2;
  
  if((ndim==3))
    cout<<"WARNING: 3dim-analysis not implemented yet - preparing for 2dim-Analysis"<<endl;

  Histo hi = Histo(*(DataObj.getGenericPixLibHisto(type)));
  histovec.push_back(hi);

  // cout<<histovec.at(0).title()<<endl;
  // cout<<histovec.at(0).name()<<endl;
  
  // no RAW_DIFF_DATA_2 histogram for 1dim, use 0th column of RAW_DIFF_DATA_1
  // there the relevant data is in the 0th x-Bin, that is, use y-dim for analysis.
  
  if(ndim==1)
    {
      Dim[0].min = (int)DataObj.getScanStart(1);
      Dim[0].max = (int)DataObj.getScanStop(1);
      Dim[0].Bins = DataObj.getScanSteps(1);
      Dim[0].BinWidth = (Dim[0].max - Dim[0].min +1)/Dim[0].Bins;
      
      Dim[1].min = (int)DataObj.getScanStart(0);
      Dim[1].max = (int)DataObj.getScanStop(0);
      Dim[1].Bins = DataObj.getScanSteps(0);
      Dim[1].BinWidth = (Dim[1].max - Dim[1].min +1)/Dim[1].Bins;     
    }
  else
    { 
      Dim[0].min = (int)DataObj.getScanStart(0);
      Dim[0].max = (int)DataObj.getScanStop(0);
      Dim[0].Bins = DataObj.getScanSteps(0);
      Dim[0].BinWidth = (Dim[0].max - Dim[0].min +1)/Dim[0].Bins;
      
      Dim[1].min = (int)DataObj.getScanStart(1);
      Dim[1].max = (int)DataObj.getScanStop(1);
      Dim[1].Bins =  DataObj.getScanSteps(1);
      Dim[1].BinWidth = (Dim[1].max - Dim[1].min +1)/Dim[1].Bins;
    }
  
  Dim[2].min = (int)DataObj.getScanStart(2);
  Dim[2].max = (int)DataObj.getScanStop(2);
  Dim[2].Bins = DataObj.getScanSteps(2);
  
  // needs to be at least 1 for entering some for-loops - otherwise no analysis done at all
  for(int i=0; i<3; i++)
    if(Dim[i].Bins==0)
      Dim[i].Bins = 1;

  if(Dim[2].Bins != 0)
    Dim[2].BinWidth = (Dim[2].max - Dim[2].min +1)/Dim[2].Bins;
  else
    Dim[2].BinWidth = 0;

  // only use pacman if entire parameter range is scanned   
  for(int i =0; i<3; i++)
    {
      if(Dim[i].pacman)
	{
	  float nextbin = (Dim[i].max+Dim[i].BinWidth);
	  //cout<<"nextbin "<<nextbin<<endl;

	  if(nextbin < Dim[i].maxpos) 
	    Dim[i].pacman=false;
	  
	  nextbin = Dim[i].min - Dim[i].BinWidth;
	  
	  if(nextbin >= Dim[i].minpos)
	    Dim[i].pacman=false;
	}      
    }

  return 0;
  
};

// looks for connected sequences in Longparam with equal values vor Shortparam.
// optimizes results in a way that only the middle values of sequences are kept
void BocScanAnalysis::RefineResults(vector<int>* longParam, vector<int>* longMaxDist, vector<int>* shortParam, vector<int>* shortMaxDist, ScanSpecs longSpecs)
{
  vector<int> tmpveclong;
  vector<int> maxdistdiff;
  vector<int> tmpvecshort;
  list<int> tmplist;
  
  vector<int> origVecLong = *(longParam);
  vector<int> origVecShort = *(shortParam);

  tmplist.clear();
  
  while(!origVecShort.empty())
    { 
      int param = origVecShort.at(0);
      // before RefineResults-Algorithm, modmaxdist should be the same for all cases.
      tmplist.clear();
 
       
      for(unsigned int i = 0; i<origVecShort.size(); i++)
	{
	  if(origVecShort.at(i) == param)
	    {
	      tmplist.push_back(origVecLong.at(i));
	    }
	}
      
      // loop backwards because length of vector is changed in loop
      // perhaps reverse iterators would work also?
      for(int i = origVecShort.size()-1; i>-1; i--)
	{
	  if(origVecShort.at(i)==param)
	    {
	      vector<int>::iterator tmpIt2 = origVecLong.begin();
	      vector<int>::iterator tmpIt = origVecShort.begin();
	   
	      for(int j = 1; j<i+1; j++)
		{
		  tmpIt2++;
		  tmpIt++;
		}
	      
	      origVecShort.erase(tmpIt); 
	      origVecLong.erase(tmpIt2);
	    }
	}
      
      tmplist.sort();
      
      // for phases, regard 0 and 25 ns as neighbours, so reorder list again if both occur in list.
      if(longSpecs.pacman)
	{
	  int counter2 = 0;
	  
	  while(counter2<longSpecs.Bins)
	    {
	      list<int>::iterator endIT2 = tmplist.end();
	      endIT2 --;
	      
	      if(*(tmplist.begin())==(((int)(*(endIT2)+ longSpecs.BinWidth))%(int)longSpecs.maxpos))
		{
		  tmplist.push_front(*endIT2);
		  tmplist.pop_back();
		  counter2++;
		}
	      else
		break;
	    }
	}

      while(!tmplist.empty())
	{
	  // for( list<int>::iterator listIt = tmplist.begin(); listIt !=tmplist.end(); listIt++)
	  // {  
	  //    cout<<(*listIt)<<endl;
	  //  }
	  
	  // counts number of values in a row
	  int counter = 0;
	  
	  list<int>::iterator endIT = tmplist.end();
	  endIT --;
	  
	  for( list<int>::iterator listIt = tmplist.begin(); listIt != endIT; listIt++)
	    {
	      list<int>::iterator nextIt = listIt;
	      nextIt++;
	     
	      if(longSpecs.pacman)
		{
		  if( (((int)((*listIt)+longSpecs.BinWidth))%(int)longSpecs.maxpos) == *(nextIt))
		    counter++;
		  else
		    break;
		}
	      else
		{
		  if((*listIt)+longSpecs.BinWidth == *(nextIt))
		    counter++;
		  else
		    break;
		}
	    }

	  for(int j =0; j<counter/2; j++)
	    tmplist.pop_front();
	  
	  tmpveclong.push_back(*(tmplist.begin()));
	  maxdistdiff.push_back(counter/2);
	  tmpvecshort.push_back(param);
	
	  tmplist.pop_front();
	  

	  // have 2 middle values if number of elements in a row is even - counting from 0 on -> counter uneven
	  if(!(counter%2==0))
	    {
	      tmpveclong.push_back(*(tmplist.begin()));
	       maxdistdiff.push_back(counter/2);
	      tmpvecshort.push_back(param);
	     
	    }
	  
	      for(int j =counter/2+1; j<counter+1; j++)
		tmplist.pop_front();
	}
    }
  

  
  // for(unsigned int z=0; z<maxdistdiff.size(); z++)
  //   cout<<"maxdistdiff "<<maxdistdiff.at(z)<<endl;


  // loop backwards because length of vector is changed in loop
  // perhaps reverse iterators would work also?
   
  
  
  for(int i = shortParam->size()-1; i>-1; i--)
    {
      bool keep = false;
      int maxdiff = 0;

      for(unsigned int j =0; j<tmpveclong.size();j++)
	{
	  if(tmpveclong.at(j)==longParam->at(i))
	    {
	      if(tmpvecshort.at(j)==shortParam->at(i))
		{
		  keep=true;
		  maxdiff=maxdistdiff.at(j);
		  break;
		}
	    }
	}
      
      // delete unwanted values
      if(keep==false)
	{
	  vector<int>::iterator tmpIt = shortParam->begin();
	  vector<int>::iterator tmpIt2 = longParam->begin();
	  vector<int>::iterator tmpIt3 = shortMaxDist->begin();
	  vector<int>::iterator tmpIt4 = longMaxDist->begin();
	  
	  for(int j = 1; j<i+1; j++)
	    {
	      tmpIt++;
	      tmpIt2++;
	      tmpIt3++;
	      tmpIt4++;	
	    }
	  
	  shortParam->erase(tmpIt); 
	  longParam->erase(tmpIt2);
	  shortMaxDist->erase(tmpIt3);
	  longMaxDist->erase(tmpIt4);
	}
      // otherwise modify modmaxdist
      else
	{
	  longMaxDist->at(i) += maxdiff; 
	}
    }
  /*  
  cout<<"shortParam:"<<endl;
  for(unsigned int z=0; z<shortParam->size(); z++)
    cout<<shortParam->at(z)<<" | ";
  cout<<endl;  
  
  cout<<"shortMaxDist:"<<endl;
  for(unsigned int z=0; z<shortMaxDist->size(); z++)
    cout<<shortMaxDist->at(z)<<" | ";
  cout<<endl;  

  cout<<"longParam:"<<endl;
  for(unsigned int z=0; z<longParam->size(); z++)
    cout<<longParam->at(z)<<" | ";
  cout<<endl;  

  cout<<"longMaxDist:"<<endl;
  for(unsigned int z=0; z<longMaxDist->size(); z++)
    cout<<longMaxDist->at(z)<<" | ";
  cout<<endl;   
  */
  return;
}

void BocScanAnalysis::shiftResults(BocScanAnalysisResults* results)
{
  vector<int>* DelayVec=0;
  vector<int>* DelayDist=0;
  vector<int>* ThresholdVec=0;
  vector<int>* ThresholdDist=0;
  float BinWidth_Delay;
  float BinWidth_Threshold;

  if(results->param[0]=="BOC_RX_DELAY")
    {
      DelayVec=&(results->bestX);
      DelayDist=&(results->modmaxdistX);
      BinWidth_Delay = Dim[0].BinWidth;
    }
  else if(results->param[1]=="BOC_RX_DELAY")
    {
      DelayVec=&(results->bestY);
      DelayDist=&(results->modmaxdistY);
      BinWidth_Delay = Dim[1].BinWidth;
    }
  else if(results->param[2]=="BOC_RX_DELAY")
    {
      DelayVec=&(results->bestZ);
      DelayDist=&(results->modmaxdistZ);
      BinWidth_Delay = Dim[2].BinWidth;
    } 
  
  if(results->param[0]=="BOC_RX_THR")
    {
      ThresholdVec=&(results->bestX);
      ThresholdDist=&(results->modmaxdistX);
      BinWidth_Threshold = Dim[0].BinWidth;
    }
 else if(results->param[1]=="BOC_RX_THR")
   {
     ThresholdVec=&(results->bestY);
     ThresholdDist=&(results->modmaxdistY);
     BinWidth_Threshold = Dim[1].BinWidth;
   }
 else if(results->param[2]=="BOC_RX_THR")
   {
     ThresholdVec=&(results->bestZ);
     ThresholdDist=&(results->modmaxdistZ);
     BinWidth_Threshold = Dim[2].BinWidth;
   }
  
  if(DelayVec!=0)
    {
      cout<<"Shifting RX-Delay values:"<<endl;
      for(unsigned int i=0; i<DelayVec->size(); i++)
	{
	  cout<<"shifting "<<DelayVec->at(i);
	  DelayVec->at(i) -= (int)((DelayDist->at(i)/3)*BinWidth_Delay);
	 
	  // for pacman
	  if(DelayVec->at(i)<0)
	    DelayVec->at(i) = 25+DelayVec->at(i); 
	  
	  cout<<" to "<< DelayVec->at(i)<<endl;
	}
    }
  // if upper threshold can be seen, do not shift to higher values.
  if((ThresholdVec!=0)&&(results->seeUpperThreshold==false))
    {
      cout<<"Shifting RX-Threshold values:"<<endl;
      for(unsigned int i=0; i<ThresholdVec->size(); i++)
	{
	  cout<<"Shifting "<<ThresholdVec->at(i);
	  ThresholdVec->at(i) += (int)((ThresholdDist->at(i)/3)*BinWidth_Threshold);
	  cout<<" to "<< ThresholdVec->at(i)<<endl;
	}
    }

  return;
}
