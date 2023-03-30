// (C)2008 S2 Games
// c_rasterbuffer.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_rasterbuffer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

/*====================
  CRasterBuffer::~CRasterBuffer
  ====================*/
CRasterBuffer::~CRasterBuffer()
{
	SAFE_DELETE_ARRAY(m_pBuffer);
}


/*====================
  CRasterBuffer::CRasterBuffer
  ====================*/
CRasterBuffer::CRasterBuffer(uint uiSize) :
m_pBuffer(K2_NEW_ARRAY(ctx_RasterBuffer, byte, uiSize))
{
}

/*====================
  CRasterBuffer::Clear
  ====================*/
void	CRasterBuffer::Clear(byte yValue, uint uiSize)
{
	MemManager.Set(m_pBuffer, 0, uiSize);
}


/*====================
  CRasterBuffer::DrawCircle

  based on http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  ====================*/
void	CRasterBuffer::DrawCircle(uint uiBufferSpan, int x0, int y0, int radius)
{
	int f(1 - radius);
	int ddF_x(0);
	int ddF_y(-2 * radius);
	int x(0);
	int y(radius);

	DrawPixel(uiBufferSpan, x0, y0 + radius);
	DrawPixel(uiBufferSpan, x0, y0 - radius);
	DrawPixel(uiBufferSpan, x0 + radius, y0);
	DrawPixel(uiBufferSpan, x0 - radius, y0);

	while (x < y) 
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x + 1;
		
		DrawPixel(uiBufferSpan, x0 + x, y0 + y);
		DrawPixel(uiBufferSpan, x0 - x, y0 + y);
		DrawPixel(uiBufferSpan, x0 + x, y0 - y);
		DrawPixel(uiBufferSpan, x0 - x, y0 - y);
		DrawPixel(uiBufferSpan, x0 + y, y0 + x);
		DrawPixel(uiBufferSpan, x0 - y, y0 + x);
		DrawPixel(uiBufferSpan, x0 + y, y0 - x);
		DrawPixel(uiBufferSpan, x0 - y, y0 - x);
	}
}


/*====================
  CRasterBuffer::DrawFilledCircle

  Draws a filled circle
  based on http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  ====================*/
void	CRasterBuffer::DrawFilledCircle(uint uiBufferSpan, int x0, int y0, int radius)
{
	int f(1 - radius);
	int ddF_x(0);
	int ddF_y(-2 * radius);
	int x(0);
	int y(radius);

	DrawPixel(uiBufferSpan, x0, y0 + radius);
	DrawPixel(uiBufferSpan, x0, y0 - radius);
	DrawHLine(uiBufferSpan, x0 - radius, x0 + radius, y0);

	while (x < y) 
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x + 1;

		DrawHLine(uiBufferSpan, x0 - x, x0 + x, y0 + y);
		DrawHLine(uiBufferSpan, x0 - x, x0 + x, y0 - y);
		DrawHLine(uiBufferSpan, x0 - y, x0 + y, y0 + x);
		DrawHLine(uiBufferSpan, x0 - y, x0 + y, y0 - x);
	}
}


/*====================
  CRasterBuffer::DrawOuterFilledCircle

  based on http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
  ====================*/
void	CRasterBuffer::DrawOuterFilledCircle(uint uiBufferSpan, int x0, int y0, int radius)
{
	int f(1 - radius);
	int ddF_x(0);
	int ddF_y(-2 * radius);
	int x(0);
	int y(radius);

	DrawHLine(uiBufferSpan, 0, uiBufferSpan - 1, y0 + radius);
	DrawHLine(uiBufferSpan, 0, uiBufferSpan - 1, y0 - radius);
	DrawHLine(uiBufferSpan, x0 + radius, uiBufferSpan - 1, y0);
	DrawHLine(uiBufferSpan, 0, x0 - radius, y0);

	while (x < y) 
	{
		if (f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}

		x++;
		ddF_x += 2;
		f += ddF_x + 1;
		
		DrawHLine(uiBufferSpan, x0 + x, uiBufferSpan - 1, y0 + y);
		DrawHLine(uiBufferSpan, 0, x0 - x, y0 + y);
		DrawHLine(uiBufferSpan, x0 + x, uiBufferSpan - 1, y0 - y);
		DrawHLine(uiBufferSpan, 0, x0 - x, y0 - y);
		DrawHLine(uiBufferSpan, x0 + y, uiBufferSpan - 1, y0 + x);
		DrawHLine(uiBufferSpan, 0, x0 - y, y0 + x);
		DrawHLine(uiBufferSpan, x0 + y, uiBufferSpan - 1, y0 - x);
		DrawHLine(uiBufferSpan, 0, x0 - y, y0 - x);
	}
}


/*====================
  CRasterBuffer::DrawLine

  Standard midpoint line algorithm
  ====================*/
void	CRasterBuffer::DrawLine(uint uiBufferSpan, int x0, int y0, int x1, int y1)
{
	int dx(x1 - x0);
	int dy(y1 - y0);
	int d(2 * dy - dx);			// Error term
	int incE(2 * dy);			// Increment used to move E
	int incNE(2 * (dy - dx));	// Increment used to move NE

	int x(x0);
	int y(y0);

	// First pixel
	DrawPixel(uiBufferSpan, x, y);

	while (x < x1)
	{
		if (d <= 0)
		{
			d += incE;
			++x;
		}
		else
		{
			d += incNE;
			++x;
			++y;
		}

		DrawPixel(uiBufferSpan, x, y);
	}
}


/*====================
  CRasterBuffer::CalcVisibiltyOctant1
  ====================*/
void	CRasterBuffer::CalcVisibiltyOctant1(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckX(m_v2Origin.x + iDistance);
	int iEndY(iEndSpan);
	byte *pPixel(&m_pBuffer[iStartSpan * m_uiBufferSpan + iCheckX]);
	const byte *pOcclusion(&m_pOcclusion[iStartSpan * m_uiBufferSpan + iCheckX]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
			
	for (int iCheckY(iStartSpan + 1); iCheckY <= iEndY; ++iCheckY)
	{
		pOcclusion += m_uiBufferSpan;
		bool bInOccluder(*pOcclusion != 0);

		pPixel += m_uiBufferSpan;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx((iCheckX - m_v2Origin.x) * 2 + 1);
				int dy((iCheckY - m_v2Origin.y) * 2 - 1);

				int iNewEndSpan = iCheckY - 1;
				int iNewEndInc1 = 2 * dy;
				int iNewEndInc2 = 2 * (dy - dx);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					++iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					++iNewEndSpan;
				}

				CalcVisibiltyOctant1(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx((iCheckX - m_v2Origin.x) * 2 - 1);
				int dy((iCheckY - m_v2Origin.y) * 2 - 1);

				iStartSpan = iCheckY;
				iStartInc1 = 2 * dy;
				iStartInc2 = 2 * (dy - dx);
				iStartError = (dy - dx);

				if (iStartError == 0)
					++iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			++iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			++iEndSpan;
		}

		if (iStartSpan <= iEndSpan)
			CalcVisibiltyOctant1(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibiltyOctant2
  ====================*/
void	CRasterBuffer::CalcVisibiltyOctant2(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckY(m_v2Origin.y + iDistance);
	int iEndX(iEndSpan);
	byte *pPixel(&m_pBuffer[iCheckY * m_uiBufferSpan + iStartSpan]);
	const byte *pOcclusion(&m_pOcclusion[iCheckY * m_uiBufferSpan + iStartSpan]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckX(iStartSpan + 1); iCheckX <= iEndX; ++iCheckX)
	{
		pOcclusion += 1;
		bool bInOccluder(*pOcclusion != 0);

		pPixel += 1;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx((iCheckX - m_v2Origin.x) * 2 - 1);
				int dy((iCheckY - m_v2Origin.y) * 2 + 1);
				
				int iNewEndSpan = iCheckX - 1;
				int iNewEndInc1 = 2 * dx;
				int iNewEndInc2 = 2 * (dx - dy);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					++iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					++iNewEndSpan;
				}

				CalcVisibiltyOctant2(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx((iCheckX - m_v2Origin.x) * 2 - 1);
				int dy((iCheckY - m_v2Origin.y) * 2 - 1);
				
				iStartSpan = iCheckX;
				iStartInc1 = 2 * dx;
				iStartInc2 = 2 * (dx - dy);
				iStartError = (dx - dy);

				if (iStartError == 0)
					++iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			++iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			++iEndSpan;
		}

		if (iStartSpan <= iEndSpan)
			CalcVisibiltyOctant2(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibiltyOctant3
  ====================*/
void	CRasterBuffer::CalcVisibiltyOctant3(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckY(m_v2Origin.y + iDistance);
	int iEndX(iEndSpan);
	byte *pPixel(&m_pBuffer[iCheckY * m_uiBufferSpan + iStartSpan]);
	const byte *pOcclusion(&m_pOcclusion[iCheckY * m_uiBufferSpan + iStartSpan]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;

	for (int iCheckX(iStartSpan - 1); iCheckX >= iEndX; --iCheckX)
	{
		pOcclusion -= 1;
		bool bInOccluder(*pOcclusion != 0);

		pPixel -= 1;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx((m_v2Origin.x - iCheckX) * 2 - 1);
				int dy((iCheckY - m_v2Origin.y) * 2 + 1);
				
				int iNewEndSpan = iCheckX + 1;
				int iNewEndInc1 = 2 * dx;
				int iNewEndInc2 = 2 * (dx - dy);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					--iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					--iNewEndSpan;
				}

				CalcVisibiltyOctant3(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx((m_v2Origin.x - iCheckX) * 2 - 1);
				int dy((iCheckY - m_v2Origin.y) * 2 - 1);
				
				iStartSpan = iCheckX;
				iStartInc1 = 2 * dx;
				iStartInc2 = 2 * (dx - dy);
				iStartError = (dx - dy);

				if (iStartError == 0)
					--iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			--iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			--iEndSpan;
		}

		if (iStartSpan >= iEndSpan)
			CalcVisibiltyOctant3(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibiltyOctant4
  ====================*/
void	CRasterBuffer::CalcVisibiltyOctant4(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckX(m_v2Origin.x - iDistance);
	int iEndY(iEndSpan);
	byte *pPixel(&m_pBuffer[iStartSpan * m_uiBufferSpan + iCheckX]);
	const byte *pOcclusion(&m_pOcclusion[iStartSpan * m_uiBufferSpan + iCheckX]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckY(iStartSpan + 1); iCheckY <= iEndY; ++iCheckY)
	{
		pOcclusion += m_uiBufferSpan;
		bool bInOccluder(*pOcclusion != 0);

		pPixel += m_uiBufferSpan;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx((m_v2Origin.x - iCheckX) * 2 + 1);
				int dy((iCheckY - m_v2Origin.y) * 2 - 1);

				int iNewEndSpan = iCheckY - 1;
				int iNewEndInc1 = 2 * dy;
				int iNewEndInc2 = 2 * (dy - dx);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					++iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					++iNewEndSpan;
				}

				CalcVisibiltyOctant4(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx((m_v2Origin.x - iCheckX) * 2 - 1);
				int dy((iCheckY - m_v2Origin.y) * 2 - 1);

				iStartSpan = iCheckY;
				iStartInc1 = 2 * dy;
				iStartInc2 = 2 * (dy - dx);
				iStartError = (dy - dx);

				if (iStartError == 0)
					++iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			++iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			++iEndSpan;
		}

		if (iStartSpan <= iEndSpan)
			CalcVisibiltyOctant4(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibiltyOctant5
  ====================*/
void	CRasterBuffer::CalcVisibiltyOctant5(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckX(m_v2Origin.x - iDistance);
	int iEndY(iEndSpan);
	byte *pPixel(&m_pBuffer[iStartSpan * m_uiBufferSpan + iCheckX]);
	const byte *pOcclusion(&m_pOcclusion[iStartSpan * m_uiBufferSpan + iCheckX]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckY(iStartSpan - 1); iCheckY >= iEndY; --iCheckY)
	{
		pOcclusion -= m_uiBufferSpan;
		bool bInOccluder(*pOcclusion != 0);

		pPixel -= m_uiBufferSpan;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx((m_v2Origin.x - iCheckX) * 2 + 1);
				int dy((m_v2Origin.y - iCheckY) * 2 - 1);

				int iNewEndSpan = iCheckY + 1;
				int iNewEndInc1 = 2 * dy;
				int iNewEndInc2 = 2 * (dy - dx);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					--iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					--iNewEndSpan;
				}

				CalcVisibiltyOctant5(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx((m_v2Origin.x - iCheckX) * 2 - 1);
				int dy((m_v2Origin.y - iCheckY) * 2 - 1);

				iStartSpan = iCheckY;
				iStartInc1 = 2 * dy;
				iStartInc2 = 2 * (dy - dx);
				iStartError = (dy - dx);

				if (iStartError == 0)
					--iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			--iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			--iEndSpan;
		}

		if (iStartSpan >= iEndSpan)
			CalcVisibiltyOctant5(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibiltyOctant6
  ====================*/
void	CRasterBuffer::CalcVisibiltyOctant6(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckY(m_v2Origin.y - iDistance);
	int iEndX(iEndSpan);
	byte *pPixel(&m_pBuffer[iCheckY * m_uiBufferSpan + iStartSpan]);
	const byte *pOcclusion(&m_pOcclusion[iCheckY * m_uiBufferSpan + iStartSpan]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckX(iStartSpan - 1); iCheckX >= iEndX; --iCheckX)
	{
		pOcclusion -= 1;
		bool bInOccluder(*pOcclusion != 0);

		pPixel -= 1;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx((m_v2Origin.x - iCheckX) * 2 - 1);
				int dy((m_v2Origin.y - iCheckY) * 2 + 1);
				
				int iNewEndSpan = iCheckX + 1;
				int iNewEndInc1 = 2 * dx;
				int iNewEndInc2 = 2 * (dx - dy);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					--iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					--iNewEndSpan;
				}

				CalcVisibiltyOctant6(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx((m_v2Origin.x - iCheckX) * 2 - 1);
				int dy((m_v2Origin.y - iCheckY) * 2 - 1);
				
				iStartSpan = iCheckX;
				iStartInc1 = 2 * dx;
				iStartInc2 = 2 * (dx - dy);
				iStartError = (dx - dy);

				if (iStartError == 0)
					--iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			--iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			--iEndSpan;
		}

		if (iStartSpan >= iEndSpan)
			CalcVisibiltyOctant6(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibiltyOctant7
  ====================*/
void	CRasterBuffer::CalcVisibiltyOctant7(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckY(m_v2Origin.y - iDistance);
	int iEndX(iEndSpan);
	byte *pPixel(&m_pBuffer[iCheckY * m_uiBufferSpan + iStartSpan]);
	const byte *pOcclusion(&m_pOcclusion[iCheckY * m_uiBufferSpan + iStartSpan]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckX(iStartSpan + 1); iCheckX <= iEndX; ++iCheckX)
	{
		pOcclusion += 1;
		bool bInOccluder(*pOcclusion != 0);

		pPixel += 1;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx((iCheckX - m_v2Origin.x) * 2 - 1);
				int dy((m_v2Origin.y - iCheckY) * 2 + 1);
				
				int iNewEndSpan = iCheckX - 1;
				int iNewEndInc1 = 2 * dx;
				int iNewEndInc2 = 2 * (dx - dy);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					++iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					++iNewEndSpan;
				}

				CalcVisibiltyOctant7(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx((iCheckX - m_v2Origin.x) * 2 - 1);
				int dy((m_v2Origin.y - iCheckY) * 2 - 1);
				
				iStartSpan = iCheckX;
				iStartInc1 = 2 * dx;
				iStartInc2 = 2 * (dx - dy);
				iStartError = (dx - dy);

				if (iStartError == 0)
					++iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			++iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			++iEndSpan;
		}

		if (iStartSpan <= iEndSpan)
			CalcVisibiltyOctant7(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibiltyOctant8
  ====================*/
void	CRasterBuffer::CalcVisibiltyOctant8(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckX(m_v2Origin.x + iDistance);
	int iEndY(iEndSpan);
	byte *pPixel(&m_pBuffer[iStartSpan * m_uiBufferSpan + iCheckX]);
	const byte *pOcclusion(&m_pOcclusion[iStartSpan * m_uiBufferSpan + iCheckX]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckY(iStartSpan - 1); iCheckY >= iEndY; --iCheckY)
	{
		pOcclusion -= m_uiBufferSpan;
		bool bInOccluder(*pOcclusion != 0);

		pPixel -= m_uiBufferSpan;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx((iCheckX - m_v2Origin.x) * 2 + 1);
				int dy((m_v2Origin.y - iCheckY) * 2 - 1);

				int iNewEndSpan = iCheckY + 1;
				int iNewEndInc1 = 2 * dy;
				int iNewEndInc2 = 2 * (dy - dx);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					--iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					--iNewEndSpan;
				}

				CalcVisibiltyOctant8(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx((iCheckX - m_v2Origin.x) * 2 - 1);
				int dy((m_v2Origin.y - iCheckY) * 2 - 1);

				iStartSpan = iCheckY;
				iStartInc1 = 2 * dy;
				iStartInc2 = 2 * (dy - dx);
				iStartError = (dy - dx);

				if (iStartError == 0)
					--iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			--iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			--iEndSpan;
		}

		if (iStartSpan >= iEndSpan)
			CalcVisibiltyOctant8(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibilty

  Uses iterative recusive shadowcasting
  ====================*/
void	CRasterBuffer::CalcVisibilty(uint uiBufferSpan, const byte *pOcclusion)
{
	// Set working variables
	m_uiBufferSpan = uiBufferSpan;
	m_pOcclusion = pOcclusion;
	m_iRadius = int(uiBufferSpan >> 1);
	m_v2Origin = CVec2i(m_iRadius, m_iRadius);

	Clear(0, m_uiBufferSpan * m_uiBufferSpan);

	DrawPixel(m_uiBufferSpan, m_v2Origin.x, m_v2Origin.y);

	//
	// Octant I (East to Northeast)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.y);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.y + 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibiltyOctant1(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant II (North to Northeast)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.x);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.x + 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibiltyOctant2(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant III (North to Northwest)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.x);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.x - 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibiltyOctant3(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant IV (West to Northwest)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.y);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.y + 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibiltyOctant4(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant V (West to Southwest)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.y);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.y - 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibiltyOctant5(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant VI (South to Southwest)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.x);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.x - 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibiltyOctant6(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant VII (South to Southeast)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.x);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.x + 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibiltyOctant7(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant VIII (East to Southeast)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.y);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.y - 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibiltyOctant8(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

#if 0
	byte *pBuffer(m_pBuffer);
	byte *pEndBuffer(m_pBuffer + m_uiBufferSpan * m_uiBufferSpan);

	for (; pBuffer != pEndBuffer; ++pBuffer, ++pOcclusion)
		*pBuffer ^= *pOcclusion == 255 ? 0 : 255;	
#endif

	//DrawLine(m_uiBufferSpan, m_v2Origin.x, m_v2Origin.y, m_v2Origin.x + m_uiRadius, m_v2Origin.y + m_uiRadius);
	//DrawLine(m_uiBufferSpan, m_v2Origin.x, m_v2Origin.y, m_v2Origin.x + m_uiRadius, m_v2Origin.y + m_uiRadius / 2);
	//DrawHLine(m_uiBufferSpan, m_v2Origin.x, m_v2Origin.x + m_uiRadius, m_v2Origin.y);
	//DrawVLine(m_uiBufferSpan, m_v2Origin.x + m_uiRadius, m_v2Origin.y, m_v2Origin.y + m_uiRadius / 2 - 2);
}


/*====================
  CRasterBuffer::CalcVisibilty2Octant1
  ====================*/
void	CRasterBuffer::CalcVisibilty2Octant1(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckX(m_v2Origin.x + iDistance);
	int iEndY(iEndSpan);
	byte *pPixel(&m_pBuffer[iStartSpan * m_uiBufferSpan + iCheckX]);
	const byte *pOcclusion(&m_pOcclusion[iStartSpan * m_uiBufferSpan + iCheckX]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
			
	for (int iCheckY(iStartSpan + 1); iCheckY <= iEndY; ++iCheckY)
	{
		pOcclusion += m_uiBufferSpan;
		bool bInOccluder(*pOcclusion != 0);

		pPixel += m_uiBufferSpan;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx(1);
				int dy(0);

				int iNewEndSpan = iCheckY - 1;
				int iNewEndInc1 = 2 * dy;
				int iNewEndInc2 = 2 * (dy - dx);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					++iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					++iNewEndSpan;
				}

				CalcVisibilty2Octant1(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx(1);
				int dy(1);

				iStartSpan = iCheckY;
				iStartInc1 = 2 * dy;
				iStartInc2 = 2 * (dy - dx);
				iStartError = (dy - dx);

				//if (iStartError == 0)
				//	++iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			++iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			++iEndSpan;
		}

		if (iStartSpan <= iEndSpan)
			CalcVisibilty2Octant1(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}

/*====================
  CRasterBuffer::CalcVisibilty2Octant2
  ====================*/
void	CRasterBuffer::CalcVisibilty2Octant2(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckY(m_v2Origin.y + iDistance);
	int iEndX(iEndSpan);
	byte *pPixel(&m_pBuffer[iCheckY * m_uiBufferSpan + iStartSpan]);
	const byte *pOcclusion(&m_pOcclusion[iCheckY * m_uiBufferSpan + iStartSpan]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckX(iStartSpan + 1); iCheckX <= iEndX; ++iCheckX)
	{
		pOcclusion += 1;
		bool bInOccluder(*pOcclusion != 0);

		pPixel += 1;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx(0);
				int dy(1);
				
				int iNewEndSpan = iCheckX - 1;
				int iNewEndInc1 = 2 * dx;
				int iNewEndInc2 = 2 * (dx - dy);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					++iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					++iNewEndSpan;
				}

				CalcVisibilty2Octant2(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx(1);
				int dy(1);
				
				iStartSpan = iCheckX;
				iStartInc1 = 2 * dx;
				iStartInc2 = 2 * (dx - dy);
				iStartError = (dx - dy);

				//if (iStartError == 0)
				//	++iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			++iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			++iEndSpan;
		}

		if (iStartSpan <= iEndSpan)
			CalcVisibilty2Octant2(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibilty2Octant3
  ====================*/
void	CRasterBuffer::CalcVisibilty2Octant3(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckY(m_v2Origin.y + iDistance);
	int iEndX(iEndSpan);
	byte *pPixel(&m_pBuffer[iCheckY * m_uiBufferSpan + iStartSpan]);
	const byte *pOcclusion(&m_pOcclusion[iCheckY * m_uiBufferSpan + iStartSpan]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;

	for (int iCheckX(iStartSpan - 1); iCheckX >= iEndX; --iCheckX)
	{
		pOcclusion -= 1;
		bool bInOccluder(*pOcclusion != 0);

		pPixel -= 1;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx(0);
				int dy(1);
				
				int iNewEndSpan = iCheckX + 1;
				int iNewEndInc1 = 2 * dx;
				int iNewEndInc2 = 2 * (dx - dy);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					--iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					--iNewEndSpan;
				}

				CalcVisibilty2Octant3(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx(1);
				int dy(1);
				
				iStartSpan = iCheckX;
				iStartInc1 = 2 * dx;
				iStartInc2 = 2 * (dx - dy);
				iStartError = (dx - dy);

				//if (iStartError == 0)
				//	--iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			--iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			--iEndSpan;
		}

		if (iStartSpan >= iEndSpan)
			CalcVisibilty2Octant3(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibilty2Octant4
  ====================*/
void	CRasterBuffer::CalcVisibilty2Octant4(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckX(m_v2Origin.x - iDistance);
	int iEndY(iEndSpan);
	byte *pPixel(&m_pBuffer[iStartSpan * m_uiBufferSpan + iCheckX]);
	const byte *pOcclusion(&m_pOcclusion[iStartSpan * m_uiBufferSpan + iCheckX]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckY(iStartSpan + 1); iCheckY <= iEndY; ++iCheckY)
	{
		pOcclusion += m_uiBufferSpan;
		bool bInOccluder(*pOcclusion != 0);

		pPixel += m_uiBufferSpan;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx(1);
				int dy(0);

				int iNewEndSpan = iCheckY - 1;
				int iNewEndInc1 = 2 * dy;
				int iNewEndInc2 = 2 * (dy - dx);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					++iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					++iNewEndSpan;
				}

				CalcVisibilty2Octant4(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx(1);
				int dy(1);

				iStartSpan = iCheckY;
				iStartInc1 = 2 * dy;
				iStartInc2 = 2 * (dy - dx);
				iStartError = (dy - dx);

				//if (iStartError == 0)
				//	++iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			++iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			++iEndSpan;
		}

		if (iStartSpan <= iEndSpan)
			CalcVisibilty2Octant4(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibilty2Octant5
  ====================*/
void	CRasterBuffer::CalcVisibilty2Octant5(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckX(m_v2Origin.x - iDistance);
	int iEndY(iEndSpan);
	byte *pPixel(&m_pBuffer[iStartSpan * m_uiBufferSpan + iCheckX]);
	const byte *pOcclusion(&m_pOcclusion[iStartSpan * m_uiBufferSpan + iCheckX]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckY(iStartSpan - 1); iCheckY >= iEndY; --iCheckY)
	{
		pOcclusion -= m_uiBufferSpan;
		bool bInOccluder(*pOcclusion != 0);

		pPixel -= m_uiBufferSpan;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx(1);
				int dy(0);

				int iNewEndSpan = iCheckY + 1;
				int iNewEndInc1 = 2 * dy;
				int iNewEndInc2 = 2 * (dy - dx);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					--iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					--iNewEndSpan;
				}

				CalcVisibilty2Octant5(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx(1);
				int dy(1);

				iStartSpan = iCheckY;
				iStartInc1 = 2 * dy;
				iStartInc2 = 2 * (dy - dx);
				iStartError = (dy - dx);

				//if (iStartError == 0)
				//	--iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			--iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			--iEndSpan;
		}

		if (iStartSpan >= iEndSpan)
			CalcVisibilty2Octant5(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibilty2Octant6
  ====================*/
void	CRasterBuffer::CalcVisibilty2Octant6(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckY(m_v2Origin.y - iDistance);
	int iEndX(iEndSpan);
	byte *pPixel(&m_pBuffer[iCheckY * m_uiBufferSpan + iStartSpan]);
	const byte *pOcclusion(&m_pOcclusion[iCheckY * m_uiBufferSpan + iStartSpan]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckX(iStartSpan - 1); iCheckX >= iEndX; --iCheckX)
	{
		pOcclusion -= 1;
		bool bInOccluder(*pOcclusion != 0);

		pPixel -= 1;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx(0);
				int dy(1);
				
				int iNewEndSpan = iCheckX + 1;
				int iNewEndInc1 = 2 * dx;
				int iNewEndInc2 = 2 * (dx - dy);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					--iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					--iNewEndSpan;
				}

				CalcVisibilty2Octant6(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx(1);
				int dy(1);
				
				iStartSpan = iCheckX;
				iStartInc1 = 2 * dx;
				iStartInc2 = 2 * (dx - dy);
				iStartError = (dx - dy);

				//if (iStartError == 0)
				//	--iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			--iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			--iEndSpan;
		}

		if (iStartSpan >= iEndSpan)
			CalcVisibilty2Octant6(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibilty2Octant7
  ====================*/
void	CRasterBuffer::CalcVisibilty2Octant7(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckY(m_v2Origin.y - iDistance);
	int iEndX(iEndSpan);
	byte *pPixel(&m_pBuffer[iCheckY * m_uiBufferSpan + iStartSpan]);
	const byte *pOcclusion(&m_pOcclusion[iCheckY * m_uiBufferSpan + iStartSpan]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckX(iStartSpan + 1); iCheckX <= iEndX; ++iCheckX)
	{
		pOcclusion += 1;
		bool bInOccluder(*pOcclusion != 0);

		pPixel += 1;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx(0);
				int dy(1);
				
				int iNewEndSpan = iCheckX - 1;
				int iNewEndInc1 = 2 * dx;
				int iNewEndInc2 = 2 * (dx - dy);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					++iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					++iNewEndSpan;
				}

				CalcVisibilty2Octant7(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx(1);
				int dy(1);
				
				iStartSpan = iCheckX;
				iStartInc1 = 2 * dx;
				iStartInc2 = 2 * (dx - dy);
				iStartError = (dx - dy);

				//if (iStartError == 0)
				//	++iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			++iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			++iEndSpan;
		}

		if (iStartSpan <= iEndSpan)
			CalcVisibilty2Octant7(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibilty2Octant8
  ====================*/
void	CRasterBuffer::CalcVisibilty2Octant8(int iDistance, int iStartSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndSpan, int iEndError, int iEndInc1, int iEndInc2)
{
	if (iDistance > m_iRadius)
		return;

	int iCheckX(m_v2Origin.x + iDistance);
	int iEndY(iEndSpan);
	byte *pPixel(&m_pBuffer[iStartSpan * m_uiBufferSpan + iCheckX]);
	const byte *pOcclusion(&m_pOcclusion[iStartSpan * m_uiBufferSpan + iCheckX]);

	bool bPrevInOccluder(*pOcclusion != 0);
	*pPixel = 255;
	
	for (int iCheckY(iStartSpan - 1); iCheckY >= iEndY; --iCheckY)
	{
		pOcclusion -= m_uiBufferSpan;
		bool bInOccluder(*pOcclusion != 0);

		pPixel -= m_uiBufferSpan;
		*pPixel = 255;

		if (bInOccluder)
		{
			if (!bPrevInOccluder)
			{
				int dx(1);
				int dy(0);

				int iNewEndSpan = iCheckY + 1;
				int iNewEndInc1 = 2 * dy;
				int iNewEndInc2 = 2 * (dy - dx);
				int iNewEndError = iNewEndInc2;

				// Move start
				if (iStartError < 0)
				{
					iStartError += iStartInc1;
				}
				else
				{
					iStartError += iStartInc2;
					--iStartSpan;
				}

				// Move new end
				if (iNewEndError <= 0)
				{
					iNewEndError += iNewEndInc1;
				}
				else
				{
					iNewEndError += iNewEndInc2;
					--iNewEndSpan;
				}

				CalcVisibilty2Octant8(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iNewEndSpan, iNewEndError, iNewEndInc1, iNewEndInc2);

				bPrevInOccluder = true;
			}
		}
		else
		{
			if (bPrevInOccluder)
			{
				int dx(1);
				int dy(1);

				iStartSpan = iCheckY;
				iStartInc1 = 2 * dy;
				iStartInc2 = 2 * (dy - dx);
				iStartError = (dy - dx);

				//if (iStartError == 0)
				//	--iStartSpan;

				bPrevInOccluder = false;
			}
		}
	}

	if (!bPrevInOccluder)
	{
		// Move start
		if (iStartError < 0)
		{
			iStartError += iStartInc1;
		}
		else
		{
			iStartError += iStartInc2;
			--iStartSpan;
		}

		// Move end
		if (iEndError <= 0)
		{
			iEndError += iEndInc1;
		}
		else
		{
			iEndError += iEndInc2;
			--iEndSpan;
		}

		if (iStartSpan >= iEndSpan)
			CalcVisibilty2Octant8(iDistance + 1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}


/*====================
  CRasterBuffer::CalcVisibilty2

  Uses iterative recusive shadowcasting (WC3 style)
  ====================*/
void	CRasterBuffer::CalcVisibilty2(uint uiBufferSpan, const byte *pOcclusion)
{
	// Set working variables
	m_uiBufferSpan = uiBufferSpan;
	m_pOcclusion = pOcclusion;
	m_iRadius = int(uiBufferSpan >> 1);
	m_v2Origin = CVec2i(m_iRadius, m_iRadius);

	Clear(0, m_uiBufferSpan * m_uiBufferSpan);

	DrawPixel(m_uiBufferSpan, m_v2Origin.x, m_v2Origin.y);

	//
	// Octant I (East to Northeast)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.y);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.y + 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibilty2Octant1(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant II (North to Northeast)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.x);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.x + 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibilty2Octant2(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant III (North to Northwest)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.x);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.x - 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibilty2Octant3(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant IV (West to Northwest)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.y);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.y + 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibilty2Octant4(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant V (West to Southwest)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.y);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.y - 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibilty2Octant5(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant VI (South to Southwest)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.x);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.x - 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibilty2Octant6(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant VII (South to Southeast)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.x);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.x + 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibilty2Octant7(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}

	//
	// Octant VIII (East to Southeast)
	//

	{

		// Start
		int iStartSpan(m_v2Origin.y);		// Start of span
		int iStartError(-m_iRadius);		// Start error term
		int iStartInc1(0);					// Increment used to move straight
		int iStartInc2(2 * -m_iRadius);		// Increment used to move diagonal

		// End
		int iEndSpan(m_v2Origin.y - 1);		// End of span
		int iEndError(m_iRadius);			// End error term
		int iEndInc1(2 * m_iRadius);		// Increment used to move straight
		int iEndInc2(0);					// Increment used to move diagonal
	
		CalcVisibilty2Octant8(1, iStartSpan, iStartError, iStartInc1, iStartInc2, iEndSpan, iEndError, iEndInc1, iEndInc2);
	}
}