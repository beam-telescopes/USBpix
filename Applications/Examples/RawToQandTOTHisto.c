////////////////////////////////////////////////////////////////////////
//// RawToQandTOTHisto: Converts .raw file from SOURCE_SCAN output	////
//// into ToT and charge histograms and a root file containing		////
//// all these histograms.  										////
//// par.root is needed from TOT_CALIB_ALT scan for the ToT			////
//// calibration.													////
////																////
//// Author:	Julia Rieger (Uni Goettingen)						////	
////			Thanks to Jens and Joern!							////
////																////
////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

using namespace std;

bool debug = false; //true if you want to see the couts for debugging

//A cluster class is introduced 

class Cluster{
	int CLS;														//Cluster size
	int sum_ToT;
	double sum_Q;
	vector<int> vcol, vrow, vLVL1;							// vectors for col, row and LVL1
	vector<int> vrowdelayed;
	vector<int> vcoldelayed;
	int delay_counter;

public:

	int getToT() const;												//Function that returns sum_ToT
	double getQ() const;											//Function that returns sum_Q
	int getCLS() const;
	vector<int> getvLVL1();	
	vector<int> getvcol();	
	vector<int> getvrow();	
	int getLVL1(int i);	
	int getcol(int i);
	int getrow(int i);
	bool getToTdelayed();
	bool getToT2delayed();
	vector<int> getvrowdelayed();
	int getrowdelayed(int i);
	vector<int> getvcoldelayed();
	int getcoldelayed(int i);
	int getdelay_counter();
	int setdelay_counter(int i);

	bool compare_hit(int LVL1, int col, int row, int ToT, int ToT2, double Q, double Q2, bool ToTdelayed, bool ToT2delayed);	//Actual clustering
	Cluster combine_cluster(int iCLS, int iToT, double dQ, vector<int> LVL1);
	void clean();
 };

void Cluster::Cluster(){
	CLS = 0;
	sum_ToT = 0;
	sum_Q = 0;
	vcol.clear();
	vrow.clear();
	vLVL1.clear();
	vrowdelayed.clear();
	vcoldelayed.clear();
	delay_counter = 0;
}

void Cluster::Cluster(int LVL1, int col, int row, int ToT, int ToT2, double Q, double Q2, bool ToTdelayed, bool ToT2delayed){
	if((ToT>=0) && (ToT<=13)){				//ToT 14 and 15 are not included in ToT sum 
		CLS = 1;
		sum_ToT = ToT;
		sum_Q = Q;
		if((ToT2>=0) && (ToT2<=13)){
			CLS = 2;
			sum_ToT += ToT2;
			sum_Q += Q2;
		}	
	}else{
		if((ToT2>=0) && (ToT2<=13)){
			CLS = 1;
			sum_ToT = ToT2;
			sum_Q = Q2;
		}else if((ToT>15) || (ToT2>15)) cout << "WARNING: Invalid ToT! " << ToT << " " << ToT2 << endl;
	}
	vcol.push_back(col);
	vrow.push_back(row);
	vLVL1.push_back(LVL1);
	if(ToTdelayed){
		vrowdelayed.push_back(row);
		vcoldelayed.push_back(col);
		delay_counter++;
	}
	if(ToT2delayed){
		vrowdelayed.push_back(row+1);
		vcoldelayed.push_back(col);
		delay_counter++;
	}
}

void Cluster::~Cluster(){
	CLS = 0;
	sum_ToT = 0;
	sum_Q = 0;
	vcol.clear();
	vrow.clear();
	vLVL1.clear();
	vrowdelayed.clear();
	vcoldelayed.clear();
	delay_counter = 0;
}


void Cluster::operator=(const Cluster& that){	
	CLS = that.getCLS();
	sum_ToT = that.getToT();
	sum_Q = that.getQ();											
	vLVL1 = that.getvLVL1();	
	vcol = that.getvcol();	
	vrow = that.getvrow();	
	vrowdelayed = that.getvrowdelayed();
	vcoldelayed = that.getvcoldelayed();
	delay_counter = that.getdelay_counter();
}


int Cluster::getToT() const{												
	return sum_ToT;
}

double Cluster::getQ() const{												
	return sum_Q;
}

int Cluster::getCLS() const{												
	return CLS;
}

vector<int> Cluster::getvLVL1() {												
	return vLVL1;
}

int Cluster::getLVL1(int i) {												
	return vLVL1[i];
}

vector<int> Cluster::getvcol() {												
	return vcol;
}

int Cluster::getcol(int i) {												
	return vcol[i];
}

vector<int> Cluster::getvrow() {												
	return vrow;
}

int Cluster::getrow(int i) {												
	return vrow[i];
}

vector<int> Cluster::getvrowdelayed() {												
	return vrowdelayed;
}

int Cluster::getrowdelayed(int i) {												
	return vrowdelayed[i];
}

vector<int> Cluster::getvcoldelayed() {												
	return vcoldelayed;
}

int Cluster::getcoldelayed(int i) {												
	return vcoldelayed[i];
}

int Cluster::getdelay_counter(){
	return delay_counter;
}

int Cluster::setdelay_counter(int i){
	delay_counter = i;
	return delay_counter;
}


// Compare hit: A cluster is defined if LVL1 is between LVL1[0]-1 and LVL1[0]+3 and if col and row are between col-1 and col+1 and row-1 and row+1 of one of the pixel which is already in the cluster
bool Cluster::compare_hit(int LVL1, int col, int row, int ToT, int ToT2, double Q, double Q2, bool ToTdelayed, bool ToT2delayed){
	bool inCluster=false;
	if(CLS!=0){
		for(int j=0; j<vLVL1.size(); j++){
			if(((LVL1<=vLVL1[0]+3) && (LVL1>=vLVL1[0]-1)) && ((col<=vcol[j]+1) && (col>=vcol[j]-1)) && ((row<=vrow[j]+2)&& (row>=vrow[j]-2))) inCluster=true;
		}
		
		if(inCluster){
			if(ToTdelayed){
				vrowdelayed.push_back(row);
				vcoldelayed.push_back(col);
				delay_counter++;
			}
			if(ToT2delayed){
				vrowdelayed.push_back(row+1);
				vcoldelayed.push_back(col);
				delay_counter++;
			}
			vcol.push_back(col);
			vrow.push_back(row);
			vLVL1.push_back(LVL1);
			if((ToT>=0) && (ToT<=13)){
				CLS++;
				sum_ToT += ToT;
				sum_Q += Q;
				if((ToT2>=0) && (ToT2<=13)){
					CLS++;
					sum_ToT += ToT2;
					sum_Q += Q2;
				}	
			}else{
				if((ToT2>=0) && (ToT2<=13)){
					CLS++;
					sum_ToT += ToT2;
					sum_Q += Q2;
				}else if((ToT>15) || (ToT2>15)) cout << "WARNING: Invalid ToT! " << ToT << " " << ToT2 << endl;
			}
			
			return true;
		}else return false;
	}else{	
		if(ToTdelayed){
			vrowdelayed.push_back(row);
			vcoldelayed.push_back(col);
			delay_counter++;
		}
		if(ToT2delayed){
			vrowdelayed.push_back(row+1);
			vcoldelayed.push_back(col);
			delay_counter++;
		}
		vcol.push_back(col);
		vrow.push_back(row);
		vLVL1.push_back(LVL1);
		if((ToT>=0) && (ToT<=13)){
			CLS++;
			sum_ToT += ToT;
			sum_Q += Q;
			if((ToT2>=0) && (ToT2<=13)){
				CLS++;
				sum_ToT += ToT2;
				sum_Q += Q2;
			}	
		}else{
			if((ToT2>=0) && (ToT2<=13)){
				CLS++;
				sum_ToT += ToT2;
				sum_Q += Q2;
			}else if((ToT>15) || (ToT2>15)) cout << "WARNING: Invalid ToT! " << ToT << " " << ToT2 << endl;
		}
			return true;
	}
}

void Cluster::clean(){
	vcol.clear();
	vrow.clear();
	vLVL1.clear();
	sum_ToT=0;
	sum_Q = 0;
	CLS = 0;
	delay_counter = 0;
	vcoldelayed.clear();
	vrowdelayed.clear();
}

Cluster Cluster::combine_cluster(int iCLS, int iToT, double dQ, vector<int> LVL1){
	for(int i=0; i<LVL1.size(); i++) vLVL1.push_back(LVL1[i]);
	CLS += iCLS;
	sum_ToT += iToT;
	sum_Q += dQ;
}

//Cluster functions end

//Histogramms which are filled
TH1F *h1 = new TH1F("h1"," ",30, 0, 30);
TH1F *h2 = new TH1F("h2"," ",30, 0, 30);
TH1F *h3 = new TH1F("h3"," ",30, 0, 30);
TH1F *h4plus = new TH1F("h4+"," ",30, 0, 30);
TH1F *hall = new TH1F("hall"," ",30, 0, 30);
TH1F *cluster = new TH1F("cluster"," ",30, 0, 30);
TH1F *LV1 = new TH1F("LV1"," ",15, 0, 15);

TH1F *Qh1 = new TH1F("Qh1"," ",50, 0, 50000);
TH1F *Qh2 = new TH1F("Qh2"," ",50, 0, 50000);
TH1F *Qh3 = new TH1F("Qh3"," ",50, 0, 50000);
TH1F *Qh4plus = new TH1F("Qh4+"," ",50, 0, 50000);
TH1F *Qhall = new TH1F("Qhall"," ",50, 0, 50000);

void fill_histo(int CLS, int ToT, vector<int> LVL1, double Q){
	if(CLS == 1){
		h1->Fill(ToT);
		Qh1->Fill(Q);
	}
	if(CLS == 2){
		h2->Fill(ToT);
		Qh2->Fill(Q);
	}
	if(CLS == 3){
		h3->Fill(ToT);
		Qh3->Fill(Q);
	}
	if(CLS >= 4){
		h4plus->Fill(ToT);
		Qh4plus->Fill(Q);
	}
	hall->Fill(ToT);
	Qhall->Fill(Q);
	cluster->Fill(CLS);
	for(int i=0; i<LVL1.size(); i++){
		LV1->Fill(LVL1[i]);
	}
}

void clear_aCluster(Cluster* aCluster){
	for(int i=0; i<100; i++){
		aCluster[i].clean();
	}
}

//compares all cluster between two external trigger if they blong to the same cluster (late hits)
void compare_cluster(Cluster* aCluster, int num_cluster){

	int coldelayed, col2, row2, rowdelayed;

	for(int i=0; i<num_cluster; i++){
		if(aCluster[i].getdelay_counter()>0){		
			coldelayed = aCluster[i].getcoldelayed(0);         
			rowdelayed = aCluster[i].getrowdelayed(0);
			for(int j=i+1; j<num_cluster; j++ ){
				for(int n=0; n<aCluster[j].getvcol().size(); n++){    
					col2 = aCluster[j].getcol(n);
					row2 = aCluster[j].getrow(n);
					if((coldelayed==col2) && (rowdelayed==row2)){
						if(aCluster[j].getCLS()==aCluster[i].getdelay_counter()){
							aCluster[i].combine_cluster(aCluster[j].getCLS(), aCluster[j].getToT(), aCluster[j].getQ(), aCluster[j].getvLVL1());
							if(debug) cout << "after combine_cluster CLS " << aCluster[i].getCLS() << " ToT " << aCluster[i].getToT() << " Q " << aCluster[i].getQ() << " col " << aCluster[i].getcol(0) <<endl;
							aCluster[j].clean();
							aCluster[i].setdelay_counter(0);
							break;
						}else{
							cout << "WARNING: Cluster could not be combined properly and are deleted!" << endl;
							aCluster[i].clean();
							aCluster[j].clean();
						}					
					}
				}
			}
		}
	}
}
	



double convertToT(int col, int row, int ToT){
	//Load histograms with parameter from TOT_CALIB_ALT scan
	TFile f("par_SCC127_new.root");

	TH1F *ParA = (TH1F*)f.Get("ParA_00");
	TH1F *ParB = (TH1F*)f.Get("ParB_00");
	TH1F *ParC = (TH1F*)f.Get("ParC_00");

	//In Par histos is entry 1,1 for pixel 0,0
	double pA = ParA -> GetBinContent(col+1, row+1); 
	double pB = ParB -> GetBinContent(col+1, row+1);
	double pC = ParC -> GetBinContent(col+1, row+1);

	//cout << ParA -> GetBinContent(col+1, row+1) << endl;

	double Q = pA + pB * ToT + pC * ToT * ToT;

	return Q;

}

void SetToTHistoStyle(TH1F* histo){
	histo->SetTitle("");
	histo->GetXaxis()->SetTitle("ToT_{CODE}");
	histo->GetYaxis()->SetTitle("Number");
	histo->SetLineWidth(2);
	histo->SetLineColor(4);
	histo->SetFillColor(4);
	histo->SetFillStyle(3004);
	histo->Draw();
	histo->Write();
}

void SetQHistoStyle(TH1F* histo){
	histo->SetTitle("");
	histo->GetXaxis()->SetTitle("Q [e]");
	histo->GetYaxis()->SetTitle("Number");
	histo->SetLineWidth(2);
	histo->SetLineColor(4);
	histo->SetFillColor(4);
	histo->SetFillStyle(3004);
	histo->Draw();
	histo->Write();
}




void RawToQandTOTHisto(){ 
	
	Cluster aCluster[100];

	FILE * ifile;
    ifile = fopen("Sr_vs_HV_SOURCE_SCAN_11.txt", "r");
	
	int inum,iLVL1,icol,irow,iToT,iToT2;
	double Q = 0;
	double Q2 = 0;
	int n = 0;
	char string[50];
	int num_cluster = 0;
	bool ToTdelayed = false;
	bool ToT2delayed = false;
	int wcounter = 0; 
		
	string s1;
	
	Cluster *dummyCluster = new Cluster();
	
	while(true){										
		
		n++;
		fgets(string, 50, ifile);
		
		if ((string[0] == 'T') && (string[1] == 'D')){			//new external trigger
			if(debug) cout << "TD" << endl;
			iLVL1=0;
			if(dummyCluster->getCLS()!=0){						// write only if there is a filled cluster
				aCluster[num_cluster] = *dummyCluster;
				num_cluster++;
				if(num_cluster > 1) compare_cluster(aCluster, num_cluster);
				
				for(int i = 0; i < num_cluster; i++){
					if(aCluster[i].getCLS()>0){
						fill_histo(aCluster[i].getCLS(), aCluster[i].getToT(), aCluster[i].getvLVL1(), aCluster[i].getQ());
						if(debug)  cout << "Filling Histo with: CLS " << aCluster[i].getCLS() << " ToT " << aCluster[i].getToT() << " Q " << aCluster[i].getQ() <<endl;
					}
				}
			}
			delete dummyCluster;
			clear_aCluster(aCluster);
			num_cluster = 0;
			Cluster *dummyCluster = new Cluster();
			continue;
		}
		if((string[0] == 'D') && (string[1] == 'H')){
			iLVL1++;	
			continue;
		}

		if((string[0] == 'D') && (string[1] == 'R')){
			sscanf (string,"%s %d %d %d %d", &s1, &icol, &irow, &iToT, &iToT2);
			if(debug) cout << "Found DR col " << icol << " row " << irow << " ToT " << iToT << " ToT2 " << iToT2 << endl;
			if(iToT == 14) ToTdelayed = true;
			else ToTdelayed = false;
			if(iToT2 == 14) ToT2delayed = true;
			else ToT2delayed = false;

			Q = convertToT(icol, irow, iToT);
			Q2 = convertToT(icol, irow+1, iToT2);
			
			if(dummyCluster->compare_hit(iLVL1, icol, irow, iToT, iToT2, Q, Q2, ToTdelayed, ToT2delayed)){
				if(debug) cout << "Compare_Hit TRUE : CLS " << dummyCluster->getCLS() << " ToT " << dummyCluster->getToT() << " Q " << dummyCluster->getQ() << endl;
				continue;
			}else{
				aCluster[num_cluster] = *dummyCluster;
				if(debug)cout << "To Histo 2 ARRAY: CLS " << aCluster[num_cluster].getCLS() << " ToT " << aCluster[num_cluster].getToT() << " Q " << aCluster[num_cluster].getQ() << " col " << aCluster[num_cluster].getcol(0) << endl;
				num_cluster++;
	
				delete dummyCluster;
				Cluster *dummyCluster = new Cluster(iLVL1, icol, irow, iToT, iToT2, Q, Q2, ToTdelayed, ToT2delayed);
				if(debug)cout << "In cluster compare_hit FALSE " << icol << " row " << irow << " ToT " << iToT << " ToT2 " << iToT2 << endl;
			}
			
		}
		if((string[0]=='E') && (string[1] == 'n')){
			cout << "# Warnings = " << wcounter << endl;
			if(debug)cout << "The End!" << endl;
			break;
		}
		
		if((string[0]=='W') && (string[1] == 'A')){
			wcounter++;
		}else continue;		
	}

//---------------------------Histograms----------------------

	TFile f("QandToTHistos.root", "RECREATE");
	gROOT->SetStyle("Plain");
	gStyle->SetOptStat(0);
	TCanvas * c[12];

	for(int i = 0; i<12; i++){
		char* name = new char[100];
		sprintf(name, "c%d", i);
		c[i] = new TCanvas(name,name,600,400);
		if(i==0){ 
			SetToTHistoStyle(h1);
			c[i]->Print("ToT_Histo_cluster1.png","png");
		}
		if(i==1){ 
			SetQHistoStyle(Qh1);	
			c[i]->Print("Q_Histo_cluster1.png","png");
		}
		if(i==2){ 
			SetToTHistoStyle(h2);
			c[i]->Print("ToT_Histo_cluster2.png","png");
		}
		if(i==3){ 
			SetQHistoStyle(Qh2);
			c[i]->Print("Q_Histo_cluster2.png","png");
		}
		if(i==4){ 
			SetQHistoStyle(h3);
			c[i]->Print("ToT_Histo_cluster3.png","png");
		}
		if(i==5){ 
			SetQHistoStyle(Qh3);
			c[i]->Print("Q_Histo_cluster3.png","png");
		}
		if(i==6){ 
			SetToTHistoStyle(h4plus);			
			c[i]->Print("ToT_Histo_cluster4plus.png","png");
		}
		if(i==7){ 
			SetQHistoStyle(Qh4plus);
			c[i]->Print("Q_Histo_cluster4plus.png","png");
		}
		if(i==8){ 
			SetToTHistoStyle(hall);
			c[i]->Print("ToT_Histo_cluster_all.png","png");
		}
		if(i==9){ 
			SetQHistoStyle(Qhall);
			c[i]->Print("Q_Histo_cluster_all.png","png");
		}
		if(i==10){ 
			SetToTHistoStyle(LV1);
			LV1->GetXaxis()->SetTitle("LVL1");
			c[i]->Print("LV1.png","png");
		}
		if(i==11){ 
			SetToTHistoStyle(cluster);
			cluster->GetXaxis()->SetTitle("Cluster size");
			//gPad->SetLog
			c[i]->Print("Cluster_size.png","png");
		}
	}
	f.Close();
	delete dummyCluster;
	fclose(ifile);
}

 


