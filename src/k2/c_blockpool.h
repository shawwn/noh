// (C)2008 S2 Games
// c_blockpool.h
//
//=============================================================================
#ifndef __C_BLOCKPOOL_H__
#define __C_BLOCKPOOL_H__

//=============================================================================
// CBlockPool
//=============================================================================
class CBlockPool
{
    byte *m_ayBuffer;
    byte *m_ayAvailable;
    uint m_uiElements;
    uint m_uiPow2BytesPerBlock;

public:
    CBlockPool() : m_ayBuffer(NULL), m_ayAvailable(NULL) { }
    ~CBlockPool() { ReleaseAll(); }

    uint    GetElements() const             { return m_uiElements; }
    uint    GetPow2BytesPerBlock() const    { return m_uiPow2BytesPerBlock; }

    void    Init(uint uiElements, uint uiPow2OfBytesPerBlock)
    {
        if (m_ayBuffer != NULL)
        {
            Console << _T("BlockPool already initialized.") << newl;
            return;
        }

        m_uiElements = uiElements;
        m_uiPow2BytesPerBlock = uiPow2OfBytesPerBlock;

        m_ayBuffer = K2_NEW_ARRAY(ctx_Nav, byte, uiElements<<uiPow2OfBytesPerBlock);
        m_ayAvailable = K2_NEW_ARRAY(ctx_Nav, byte, uiElements);
        memset(m_ayAvailable, 0xff, sizeof(byte) * uiElements);
    };

    void    ReleaseAll()
    {
        SAFE_DELETE_ARRAY(m_ayBuffer);
        SAFE_DELETE_ARRAY(m_ayAvailable);
    }

    template <typename _T>
    _T  NewLongTerm(uint uiBlocks)
    {
        ushort unSize(0), unLastAvail(USHRT_MAX);

        for (uint i(0); i < m_uiElements; ++i)
        {
            if (m_ayAvailable[i])
            {
                unLastAvail = i;
                if (++unSize >= uiBlocks)
                {
                    while (unSize--)
                        m_ayAvailable[i--] = 0;

                    return reinterpret_cast<_T>(&m_ayBuffer[++i << m_uiPow2BytesPerBlock]);
                }
            }
            else
            {
                unLastAvail = USHRT_MAX;
                unSize = 0;
            }
        }

        // failed to allocate a block of the requested size
        return NULL;
    };

    template <typename _T>
    _T  NewShortTerm(uint uiBlocks)
    {
        ushort unSize(0), unLastAvail(USHRT_MAX);

        for (int i(m_uiElements - 1); i >= 0; --i)
        {
            if (m_ayAvailable[i])
            {
                unLastAvail = i;
                if (++unSize >= uiBlocks)
                {
                    while (unSize--)
                        m_ayAvailable[i++] = 0;

                    return reinterpret_cast<_T>(&m_ayBuffer[unLastAvail << m_uiPow2BytesPerBlock]);
                }
            }
            else
            {
                unLastAvail = USHRT_MAX;
                unSize = 0;
            }
        }

        // failed to allocate a block of the requested size
        return NULL;
    };

    template <typename _T>
    void    Free(_T pDealloc, uint uiBlocks)
    {
        size_t offset(reinterpret_cast<byte *>(pDealloc) - m_ayBuffer);
        ushort unOffset((ushort)(offset >> m_uiPow2BytesPerBlock));

        while (uiBlocks--)
            m_ayAvailable[unOffset++] = 0xff;
    };
};
//=============================================================================

#endif // __C_BLOCKPOOL_H__
