Simple program+ROOT script to extract and plot single events of a source scan.

Flow:
- First:  Output from raw data file is created. Formatting:
          col row trigger BCID tot
- Second: A root macro can be used to plot the Hitmap.

HOWTO:
0. qmake followed by (n)make compiles the program
1. type Data_Conversion in console and follow usage instruction
2. plot output data via: root -l -q -b plotHitmapTrigger.C, but specify filename
   and number of triggers in root script before running

EXAMPLE:
   goto program folder in the console
   type nmake
   type main test.raw out.txt
   type root -l -q -b plotHitmapTrigger.C
   stop by pressing CRTL-C
