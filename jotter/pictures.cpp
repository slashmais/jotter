
#include "pictures.h"

#define  IMAGEFILE  <jotter/jotter.iml>
#define  IMAGECLASS JPics
#include <Draw/iml.h>

#include <utilfuncs/utilfuncs.h>

Image GetPic(int npic)
{
	switch(npic)
	{
		case JOTTERICON:		return JPics::jottericon(); break;
		//case PICxxx:			return FSFindPics::PicXXX(); break;
	}
	return Image();
}

/*
//------------------------------------------------
void er_contract(Image &pic, int W)
{
	if (pic.IsNullInstance()||(W<=0)) return;
	int x, y, i, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(W, h);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (y=0;y<h;y++)
	{
		for (x=0;x<w;x++)
		{
			i=(x*W)/w;
			pib[(y*W)+i]=pic[y][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_extend(Image &pic, int W) //widening pic
{
	if (pic.IsNullInstance()||(W<=0)) return;
	int x, i, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(W, h);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (j=0;j<h;j++)
	{
		for (i=0;i<W;i++)
		{
			x=(i*w)/W;
			pib[(j*W)+i]=pic[j][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_shorten(Image &pic, int H) //shortening pic
{
	if (pic.IsNullInstance()||(H<=0)) return;
	int x, y, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(w, H);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (y=0;y<h;y++) //some source-pixels will be go to same target-pixels (many src to 1 tgt)
	{
		for (x=0;x<w;x++)
		{
			j=(y*H)/h;
			pib[(j*w)+x]=pic[y][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_highten(Image &pic, int H) //lengthening pic
{
	if (pic.IsNullInstance()||(H<=0)) return;
	int y, i, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(w, H);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (j=0;j<H;j++) //some target-pixels will be from same source-pixels (1 src to many tgt)
	{
		for (i=0;i<w;i++)
		{
			y=(j*h)/H;
			pib[(j*w)+i]=pic[y][i];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_shrink(Image &pic, int W, int H)
{
	if (pic.IsNullInstance()||(W<=0)||(H<=0)) return;
	int x, y, i, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(W, H);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (y=0;y<h;y++) //some source-pixels will be go to same target-pixels (many src to 1 tgt)
	{
		for (x=0;x<w;x++)
		{
			i=(x*W)/w;
			j=(y*H)/h;
			pib[(j*W)+i]=pic[y][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_grow(Image &pic, int W, int H)
{
	if (pic.IsNullInstance()||(W<=0)||(H<=0)) return;
	int x, y, i, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(W, H);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (j=0;j<H;j++)
	{
		for (i=0;i<W;i++)
		{
			x=(i*w)/W;
			y=(j*h)/H;
			pib[(j*W)+i]=pic[y][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void elastic_resize(Image &pic, int to_width, int to_height)
{
	//idea from: [dev_hold]tapov::void TAMap::PaintPic(Draw &drw, TATopic &T, int CX, int CY)
	//gives even spreading-out / stretching / squashing of picture - not reversible
	//((can make a class that retains copy of original pic then it can be sized at will))

	/ *
		for each pixel of target, fetch corresponding pixel from source
		enlarging => same source pixel can fill several target positions
		shrinking => some source pixels will be lost
	* /

	int pw=pic.GetWidth();
	int ph=pic.GetHeight();
	if ((pw==to_width)&&(ph==to_height)) return;
	if (pw<to_width)
	{
		if (ph<to_height) er_grow(pic, to_width, to_height);
		else if (ph==to_height) er_extend(pic, to_width);
		else { er_extend(pic, to_width); er_shorten(pic, to_height); }
	}
	else if (pw==to_width)
	{
		if (ph<to_height) er_highten(pic, to_height);
		else if (ph>to_height) er_shorten(pic, to_height);
	}
	else //pw>to_width
	{
		if (ph<to_height) { er_highten(pic, to_height); er_contract(pic, to_width); }
		else if (ph==to_height) er_contract(pic, to_width);
		else er_shrink(pic, to_width, to_height);
	}
}

*/