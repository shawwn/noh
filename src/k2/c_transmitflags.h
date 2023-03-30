// (C)2008 S2 Games
// c_transmitflags.h
//
//=============================================================================
#ifndef __C_TRANSMITFLAGS_H__
#define __C_TRANSMITFLAGS_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_utils.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CTransmitFlags
//=============================================================================
template <uint BUFFER_SIZE>
class CTransmitFlags
{
private:
	uint	m_pBuffer[BUFFER_SIZE];
	uint	m_uiLength;
	uint	m_uiNumFields;

public:
	~CTransmitFlags() {}
	CTransmitFlags() : m_uiLength(0), m_uiNumFields(0) {}
	CTransmitFlags(uint uiNumFields) : m_uiLength(0), m_uiNumFields(0) { SetNumFields(uiNumFields); }

	bool	IsFieldSet(uint uiIndex) const;
	void	SetField(uint uiIndex);
	void	SetAllFields();
	void	Clear();

	void	SetNumFields(uint uiNumFields);
	uint	GetNumFields() const			{ return m_uiNumFields; }

	void		ReadTransmitFlags(const CBufferBit &buffer);
	void		WriteTransmitFlags(CBufferBit &cBuffer) const;
};
//=============================================================================

//=============================================================================
// Inline functions
//=============================================================================

/*====================
  CTransmitFlags::SetNumFields
  ====================*/
template <uint BUFFER_SIZE>
inline
void	CTransmitFlags<BUFFER_SIZE>::SetNumFields(uint uiNumFields)
{
	m_uiNumFields = uiNumFields;

	uint uiNumGroups(CEIL_MULTIPLE<32>(uiNumFields) >> 5);

	// Grow the transmit flags vector, if necessary
	if (uiNumGroups > m_uiLength)
	{
		while (m_uiLength < uiNumGroups && m_uiLength != BUFFER_SIZE)
			m_pBuffer[m_uiLength++] = 0;
	}
	else
	{
		m_uiLength = uiNumGroups;
	}
}


/*====================
  CTransmitFlags::IsFieldSet
  ====================*/
template <uint BUFFER_SIZE>
inline
bool	CTransmitFlags<BUFFER_SIZE>::IsFieldSet(uint uiIndex) const
{
	uint uiGroup(uiIndex >> 5);

	if (uiGroup < m_uiLength)
		return ((m_pBuffer[uiGroup] & (1 << (uiIndex & 31))) != 0);
	else
		return false;
}


/*====================
  CTransmitFlags::SetField
  ====================*/
template <uint BUFFER_SIZE>
inline
void	CTransmitFlags<BUFFER_SIZE>::SetField(uint uiIndex)
{
	uint uiGroup(uiIndex >> 5);

	// Set the transmit flag bit
	if (uiGroup < m_uiLength)
		m_pBuffer[uiGroup] |= (1 << (uiIndex & 31));
}


/*====================
  CTransmitFlags::SetAllFields
  ====================*/
template <uint BUFFER_SIZE>
inline
void	CTransmitFlags<BUFFER_SIZE>::SetAllFields()
{
	for (uint ui(0); ui < m_uiLength; ++ui)
		m_pBuffer[ui] = uint(-1);
}


/*====================
  CTransmitFlags::SetAllFields
  ====================*/
template <uint BUFFER_SIZE>
inline
void	CTransmitFlags<BUFFER_SIZE>::Clear()
{
	for (uint ui(0); ui < m_uiLength; ++ui)
		m_pBuffer[ui] = 0;
}

#include "c_transmitflags.cpp"
//=============================================================================

#endif	//__C_TRANSMITFLAGS_H__
