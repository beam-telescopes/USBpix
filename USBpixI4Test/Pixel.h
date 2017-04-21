// Class to calculate pixel row/col from pixelnumber and other way round. 



// Update for FE-I4 needed !!!!!
 










#ifndef PIXEL_H
#define PIXEL_H



class Pixel
{

public:
	Pixel();
	~Pixel();


	void PixNrToDCRow(int pixel, int &row, int &DC);
	void PixNrToColRow(int pixel, int &row, int &col);
	void PixNrToColRowDataRB(int pixel, int &row, int &col);
	void PixNrToArray(int pixnr, int &index_nr, int &select_val);
	void PixColRowToPixNr(int col, int row, int &pixnr);
	int PixColRowToPixInt(int col, int row/*, int &pixint*/);
	int SolveEmitVal(int val);

private:


};



#endif
