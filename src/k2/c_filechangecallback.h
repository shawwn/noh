// (C)2010 S2 Games
// c_filechangecallback.h
//
//=============================================================================
#ifndef __C_FILECHANGECALLBACK_H__
#define __C_FILECHANGECALLBACK_H__

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// IFileChangeCallback
//=============================================================================
class IFileChangeCallback
{
private:
    tstring             m_sPath;
    bool                m_bExecuting;

public:
    K2_API virtual ~IFileChangeCallback() {}
    K2_API IFileChangeCallback(const tstring &sPath);

    virtual void        Execute() = 0;

    const tstring&      GetPath()       { return m_sPath; }


    bool    TryExecute()
    {
        if (!m_bExecuting)
        {
            m_bExecuting = true;
            return true;
        }
        return false;
    }


    void    AfterExecute()
    {
        m_bExecuting = false;
    }
};
//=============================================================================

#endif //__C_FILECHANGECALLBACK_H__

