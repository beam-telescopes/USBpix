#include "PixelDataContainer.h"

PixelDataContainer::PixelDataContainer(const char *name, const char *path)
{
  // name on the display and file path
  if(name!=0)
    m_dtname = name;
  else
    m_dtname = "";
  if(path!=0)
    m_pname  = path;
  else
    m_pname  = "";
  m_fullpath = m_pname;

  // split file name into bare name and rest of path
  int i = m_pname.find_last_of("/");
  if(i>0){
    m_pname.erase(0,i+1);
    m_fullpath.erase(i+1,m_fullpath.length()-i);
  }

  // default is FE-I4; leave it to inh. classes to overwrite dynamically
  m_nRows = 336;
  m_nCols = 80;
  m_nFe = 4;
  m_nFeRows = 2;
}
void PixelDataContainer::PixXY(int chip, int col, int row, float *xval, float *yval){

  if(chip<(m_nFe/m_nFeRows)){
    *xval = (float)(chip*m_nCols+col);
    *yval = (float)row;
  } else{
    *xval = (float)(m_nCols-1-col + m_nCols*(m_nFe-1-chip));
    *yval = (float)(2*m_nRows-1-row);
  }

  return;
}
int PixelDataContainer::PixXYtoInd(int xval, int yval){
  int row, col, chip;
  PixXYtoCCR(xval,yval,&chip,&col,&row);
  return PixIndex(chip, col, row);
}
void PixelDataContainer::PixXYtoCCR(int xval, int yval, int *chip, int *col, int *row){
  if(yval<m_nRows){
    *row  = yval;
    *col  = xval%m_nCols;
    *chip = xval/m_nCols;
  } else{
    *row  = 2*m_nRows-1- yval;
    *col  = m_nCols-1  - xval%m_nCols;
    *chip = m_nFe-1 - xval/m_nCols;
  }

  if(m_nRows==336) // FE-I4: rows are flipped
    *row = m_nRows - 1 - *row;
  return;
}
void  PixelDataContainer::PixXYtoBins(float xval, float yval, int &binx, int &biny)
{// added by Aldo returns the binx and biny that corresponds to
 // the xval and yval

  binx = (int)xval + 1;
  biny = (int)yval + 1;
  
  return;
}
void  PixelDataContainer::PixCCRtoBins(int chip, int col, int row,int &binx, int &biny){
  float xval, yval;
  PixXY(chip,col,row,&xval,&yval);
  PixXYtoBins(xval, yval, binx, biny);
  return;
}
int PixelDataContainer::Pixel_Type(int col, int row)
{

  if (( col > 0) && ( col < (m_nCols-1))){
    if (m_nRows==336){ // don't have ganged pixels, but check for VNCAP pixel of FE-I4A
      if(col==3 || col==4 || col==13 || col==14 || col==23 || col==24 || col==27 || col==28 || col==31 || col==32 || col==35 || 
	 col==36 || col==45 || col==46 || col==47 || col==48 || col==49 || col==50 || col==51 || col==52)
	return 6;
      else
	return 0;
    }else{
      if(row == 152 || row == 154 || row == 156 || row == 158)
	return 5; // inter-ganged pixel
      else if (row == 153 || row == 155 || row == 157 || row == 159)
	return 2; // ganged pixel
      else
	return 0; // normal pixel
    }
  } else {
    if (row >= 152 && m_nRows!=336)
      return 3; // long ganged
    else
      return 1; // long
  }
}
const char* PixelDataContainer::Pixel_TypeName(int type)
{// returns the name of the pixel type
  switch(type){
  case 0: //normal
    return "NORMAL";
  case 1: // long
    return "LONG";
  case 2: // ganged
    return "GANGED";
  case 3: // long ganged
    return "LONG GANGED";
  case 4: //all
    return "ALL";
  case 5: // inter-ganged
    return "INTER-GANGED";
  case 6: // inter-ganged
    return "VNCAP";
  default: //error
    return "Not Implemented";
  }
}
