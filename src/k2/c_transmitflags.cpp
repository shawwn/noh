// (C)2008 S2 Games
// c_transmitflags.cpp
//
// Included from c_transmitflags.h
//=============================================================================

/*====================
  CEntitySnapshot::ReadTransmitFlags
  ====================*/
template <uint BUFFER_SIZE>
void    CTransmitFlags<BUFFER_SIZE>::ReadTransmitFlags(const CBufferBit &cBuffer)
{
    Clear();

    // Only one byte if we have 8 or less fields
    if (m_uiNumFields <= 8)
    {
        uint uiFlags(cBuffer.ReadBits(m_uiNumFields));

        // Setup the transmit flag tree
        for (uint uiField(0); uiField < m_uiNumFields; ++uiField)
            if (uiFlags & BIT(uiField))
                SetField(uiField);

        return;
    }

    // First level
    if (cBuffer.ReadBit())
    {
        const uint uiTreeWidth(static_cast<uint>(M_CeilPow2(static_cast<int>(m_uiNumFields))));
        int iNumBytes(CEIL_MULTIPLE<4>(uiTreeWidth) >> 2);

        byte ayTransmitFlagTree[64];
        memset(ayTransmitFlagTree, 0, iNumBytes * sizeof(byte));

        // Read the transmit flag tree
        uint uiNumLevels(M_Log2(uiTreeWidth) + 1);

        K2_SetVectorBit(ayTransmitFlagTree, 1);

        // Read other levels
        for (uint uiLevel(1); uiLevel < uiNumLevels; ++uiLevel)
        {
            uint uiLevelStart(M_Power(2, uiLevel));
            uint uiLevelEnd(uiLevelStart << 1);

            for (uint uiPos(uiLevelStart); uiPos != uiLevelEnd; uiPos += 2)
            {
                if (!K2_IsParentTrue(ayTransmitFlagTree, uiPos))
                    continue;

                if (cBuffer.ReadBit())
                {
                    if (cBuffer.ReadBit())
                    {
                        K2_SetVectorBit(ayTransmitFlagTree, uiPos); // Left
                        K2_SetVectorBit(ayTransmitFlagTree, uiPos + 1); // Right
                    }
                    else
                    {
                        K2_SetVectorBit(ayTransmitFlagTree, uiPos + 1); // Right
                    }
                }
                else
                {
                    K2_SetVectorBit(ayTransmitFlagTree, uiPos); // Left
                }
            }
        }

        for (uint uiField(0); uiField < m_uiNumFields; ++uiField)
            if (K2_GetVectorBit(ayTransmitFlagTree, uiTreeWidth + uiField))
                SetField(uiField);
    }
}


/*====================
  CEntitySnapshot::WriteTransmitFlags
  ====================*/
template <uint BUFFER_SIZE>
void    CTransmitFlags<BUFFER_SIZE>::WriteTransmitFlags(CBufferBit &cBuffer) const
{
    const uint uiTreeWidth(static_cast<uint>(M_CeilPow2(static_cast<int>(m_uiNumFields))));
    int iNumBytes(CEIL_MULTIPLE<4>(uiTreeWidth) >> 2);

    // Send raw fields if we have 8 or less
    if (m_uiNumFields <= 8)
    {
        byte yFlags(0);

        // Setup the transmit flag tree
        for (uint uiField(0); uiField < m_uiNumFields; ++uiField)
            if (IsFieldSet(uiField))
                yFlags |= BIT(uiField);

        cBuffer.WriteBits(yFlags, m_uiNumFields);
        return;
    }

    byte ayTransmitFlagTree[64];
    memset(ayTransmitFlagTree, 0, iNumBytes * sizeof(byte));

    // Setup the transmit flag tree
    for (uint uiField(0); uiField < m_uiNumFields; ++uiField)
        if (IsFieldSet(uiField))
            K2_SetTreeBit(ayTransmitFlagTree, uiTreeWidth, uiField);

    if (!K2_GetVectorBit(ayTransmitFlagTree, 1))
    {
        // Empty tree
        cBuffer.WriteBit(0);
    }
    else
    {
        // Write the transmit flag tree
        uint uiNumLevels(M_Log2(uiTreeWidth) + 1);

        // First level
        cBuffer.WriteBit(1);

        // Write the other levels
        for (uint uiLevel(1); uiLevel < uiNumLevels; ++uiLevel)
        {
            uint uiLevelStart(1 << uiLevel);
            uint uiLevelEnd(uiLevelStart << 1);

            for (uint uiPos(uiLevelStart); uiPos != uiLevelEnd; uiPos += 2)
            {
                if (!K2_IsParentTrue(ayTransmitFlagTree, uiPos))
                    continue;

                uint uiQuad(K2_GetVectorQuad(ayTransmitFlagTree, uiPos));

                if (uiQuad == 1) // left and !right
                    cBuffer.WriteBit(0);
                else if (uiQuad == 2) // !left and right
                    cBuffer.WriteBits(1, 2); // 1,0
                else 
                    cBuffer.WriteBits(3, 2); // 1,1
            }
        }
    }
}
