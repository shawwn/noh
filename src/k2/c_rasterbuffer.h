// (C)2008 S2 Games
// c_rasterbuffer.h
//
//=============================================================================
#ifndef __C_RASTERBUFFER_H__
#define __C_RASTERBUFFER_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CRasterBuffer
//=============================================================================
class CRasterBuffer
{
private:
	byte	*m_pBuffer;
	uint	m_uiSize;

	// Working variables
	uint		m_uiBufferSpan;
	int			m_iRadius;
	CVec2i		m_v2Origin;
	const byte	*m_pOcclusion;

	// Recursive shadowcasting per octant
	void	CalcVisibiltyOctant1(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibiltyOctant2(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibiltyOctant3(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibiltyOctant4(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibiltyOctant5(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibiltyOctant6(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibiltyOctant7(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibiltyOctant8(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);

	// Recursive shadowcasting per octant (WC3 Style)
	void	CalcVisibilty2Octant1(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibilty2Octant2(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibilty2Octant3(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibilty2Octant4(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibilty2Octant5(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibilty2Octant6(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibilty2Octant7(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);
	void	CalcVisibilty2Octant8(int uiDistance, int iStartSpan, int iEndSpan, int iStartError, int iStartInc1, int iStartInc2, int iEndError, int iEndInc1, int iEndInc2);

	CRasterBuffer();

public:
	K2_API	~CRasterBuffer();
	K2_API	CRasterBuffer(uint uiSize);

	byte*			GetBuffer()		{ return m_pBuffer; }

	K2_API void		Clear(byte yValue, uint uiSize);
	inline void		ClearPixel(uint uiBufferSpan, int x, int y);

	inline void		DrawPixel(uint uiBufferSpan, int x, int y);
	inline void		DrawHLine(uint uiBufferSpan, int x0, int x1, int y);
	inline void		DrawVLine(uint uiBufferSpan, int x, int y0, int y1);
	K2_API void		DrawCircle(uint uiBufferSpan, int x0, int y0, int radius);
	K2_API void		DrawFilledCircle(uint uiBufferSpan, int x0, int y0, int radius);
	K2_API void		DrawOuterFilledCircle(uint uiBufferSpan, int x0, int y0, int radius);
	K2_API void		DrawLine(uint uiBufferSpan, int x0, int y0, int x1, int y1);

	K2_API void		CalcVisibilty(uint uiBufferSpan, const byte *pOcclusion);
	K2_API void		CalcVisibilty2(uint uiBufferSpan, const byte *pOcclusion);
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CRasterBuffer::DrawPixel
  ====================*/
inline
void	CRasterBuffer::DrawPixel(uint uiBufferSpan, int x, int y)
{
	m_pBuffer[y * uiBufferSpan + x] = 255;
}


/*====================
  CRasterBuffer::ClearPixel
  ====================*/
inline
void	CRasterBuffer::ClearPixel(uint uiBufferSpan, int x, int y)
{
	m_pBuffer[y * uiBufferSpan + x] = 0;
}


/*====================
  CRasterBuffer::DrawHLine
  ====================*/
inline
void	CRasterBuffer::DrawHLine(uint uiBufferSpan, int x0, int x1, int y)
{
	byte *p(&m_pBuffer[y * uiBufferSpan + x0]);
	byte *pEnd(&m_pBuffer[y * uiBufferSpan + x1 + 1]);

	for (; p != pEnd; ++p)
		*p = 255;
}


/*====================
  CRasterBuffer::DrawVLine
  ====================*/
inline
void	CRasterBuffer::DrawVLine(uint uiBufferSpan, int x, int y0, int y1)
{
	byte *p(&m_pBuffer[y0 * uiBufferSpan + x]);
	byte *pEnd(&m_pBuffer[(y1 + 1) * uiBufferSpan + x]);

	for (; p != pEnd; p += uiBufferSpan)
		*p = 255;
}
//=============================================================================

#endif //__C_RASTERBUFFER_H__
