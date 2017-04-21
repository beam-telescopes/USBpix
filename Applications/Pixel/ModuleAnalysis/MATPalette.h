#ifndef MATPAL
#define MATPAL

#include <TPaletteAxis.h>

class RMain;
/*  Try to do the same thing to TPaletteAxis
*/

class MATPaletteAxis: public TPaletteAxis
{
 public:
   MATPaletteAxis(RMain *rootparnt=0);
   MATPaletteAxis(MATPaletteAxis&, RMain *rootparnt=0);
   ~MATPaletteAxis();
   void ExecuteEvent(Int_t event, Int_t px, Int_t py);
   void SetMinMax(int new_Minimum, int new_Maximum); // *MENU* set the minimum and maximum of a mulit-module plot
 private:
   RMain *m_parnt;
   // ROOT class definition, needed to make menu work
   ClassDef(MATPaletteAxis,1);
};

#endif // MATPAL
