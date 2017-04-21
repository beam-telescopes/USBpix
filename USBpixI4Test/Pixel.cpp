#ifdef __VISUALC__
#include "stdafx.h"
#endif
#include "defines.h"
#include "Pixel.h"

Pixel::Pixel()
{
}

Pixel::~Pixel()
{
}

void Pixel::PixNrToColRow(int pixel, int &row, int &col)
{
	col = (pixel - 1) / 336;

	if (0x01 & col)
		row = 335 - ((pixel - 1) % 336);
	else 
		row = (pixel - 1)  % 336;
}

void Pixel::PixNrToColRowDataRB(int pixel, int &row, int &col)
{
	col = ((pixel - 1) / 336) + 1;

	if (0x01 & col)
		row = 336 - ((pixel - 1) % 336);
	else 
		row = ((pixel - 1)  % 336) + 1;
}


void Pixel::PixNrToDCRow(int pixel, int &row, int &DC)
{
	DC = (pixel - 1) / 672;

	if (0x01 & DC)
		row = 672 - ((pixel - 1) % 672);
	else 
		row = (pixel - 1)  % 672;
}

void Pixel::PixNrToArray(int pixnr, int &index_nr, int &value)
{
	int bit_nr = 0;

	index_nr = PIXEL32 - ((pixnr - 1) / 32);
	bit_nr = pixnr % 32;
	value = 0x00000001 << (bit_nr - 1);
}

void Pixel::PixColRowToPixNr(int col, int row, int &pixnr)
{
	if (0x01 & col)
		pixnr = (col * 336) + (335 - row) + 1;
	else
		pixnr = (col * 336) + row + 1;
}

int Pixel::PixColRowToPixInt(int col, int row/*, int &pixint*/)
{
	int pixnr = 0;
	int pixint = 0;
	PixColRowToPixNr(col, row, pixnr);
	
	pixint = (pixnr / 32) + PIXEL26880;
	return pixint;
}

int Pixel::SolveEmitVal(int val)
{
	int emitx;

	if ((val == 0x00000001))
		emitx = 2;
	else if (val == 0xffffffff)
		emitx = 1;
	else 
		emitx = 0;
	return emitx;
}

