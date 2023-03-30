// (C)2010 S2 Games
// c_restorevalue.h
//
//=============================================================================
#ifndef __C_RESTOREVALUE_H__
#define __C_RESTOREVALUE_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

// CRestoreValue is a utility which sets a variable back to its previous value when the
// current scope ends.  For example this is useful if you are setting a bool
// to "true" at the top of some function, and need to set it to "false" when the
// function exits:
//
//  void    CResourceManager::Reload(ResHandle hResource)
//  {
//      ....
//      try
//      {
//          m_bReloading = true;
//
//          .... exceptions might be thrown here ....
//
//          m_bReloading = false;
//      }
//      catch (CException& ex)
//      {
//          ex.Process(_T("CResourceManager::Reload() - "), NO_THROW);
//      }
//  }
//
// If an exception is thrown, "m_bReloading" won't be set back to false.
//
// "Well," you say, "I'll fix it like THIS!  Watch!"
//
//  void    CResourceManager::Reload(ResHandle hResource)
//  {
//      ....
//      try
//      {
//          m_bReloading = true;
//
//          .... exceptions might be thrown here ....
//      }
//      catch (CException& ex)
//      {
//          ex.Process(_T("CResourceManager::Reload() - "), NO_THROW);
//      }
//
//      // even if an exception is thrown, m_bReloading will still be correctly
//      // set to its final value... Or will it?  (Nope, read on.)
//      m_bReloading = false;
//  }
//
// Consider what would happen if:
//  CResourceManager::Reload(ResHandle hResource)
//      m_bReloading = true;
//      .... calls the method ....
//      CResourceManager::Register(tstring sPath)
//          .... which then clears a flag and calls into Reload again ....
//          CResourceManager::Reload(ResHandle hResource)
//              m_bReloading = true;
//              .... 
//              m_bReloading = false; // BUG!  We're still reloading!
//          .... m_bReloading is incorrectly 'false' here ....
//      m_bReloading = false;
//
// So, you would fix that by storing the "previous value" of m_bReloading,
// then setting m_bReloading = bPrevReloading...
//
//  void    CResourceManager::Reload(ResHandle hResource)
//  {
//      bool bPrevReloading(m_bReloading);
//      ....
//      try
//      {
//          m_bReloading = true;
//
//          .... exceptions might be thrown here ....
//      }
//      catch (CException& ex)
//      {
//          ex.Process(_T("CResourceManager::Reload() - "), NO_THROW);
//      }
//
//      // even if an exception is thrown, and even if Reload() is called recursively,
//      // m_bReloading will still be correctly set to its final value... Or will it?
//      // (Maybe not in the future!  Read on!)
//      m_bReloading = bPrevReloading;
//  }
//
// Consider what would happen if a different programmer comes along and adds a "return"
// statement in the middle of that try { .... } block?  m_bReloading would be "true"
// even after the Reload() function returns!  This is the worst kind of bug... unless
// it causes some easily-noticeable problem, this bug would likely manifest itself 
// AFTER patching with it, possibly when viewing old replays (for example).
//
// ==================
// Solution?  CRestoreValue!
// ==================
//
// CRestoreValue handles ALL of those cases, and in a much cleaner way:
// 
//  void    CResourceManager::Reload(ResHandle hResource)
//  {
//      try
//      {
//          CRestoreValue<bool> cSetReloading(m_bReloading, true);
//
//          .... exceptions might be thrown here ....
//      }
//      catch (CException& ex)
//      {
//          ex.Process(_T("CResourceManager::Reload() - "), NO_THROW);
//      }
//  }
//
// And that's it!  So simple!
//
//=============================================================================
// CRestoreValue
//=============================================================================
template<typename T>
class CRestoreValue
{
private:
    T&      m_cVariable;
    T       m_cPrevValue;

public:
    // store the previous value of a variable, then it to the specified value.
    // When our scope ends, set the variable back to its previous value.
    //
    // For example:
    //
    //  CRestoreValue<bool> cLoading(m_bLoading, true);
    //
    CRestoreValue(T& cVariable, T cSetValue)
        : m_cVariable(cVariable)
        , m_cPrevValue(cVariable) // store the previous value of the variable.
    {
        // set our variable to the value you specify.
        m_cVariable = cSetValue;
    }

    // store the previous value of a variable (without setting it to a new value).
    // When our scope ends, set the variable back to its previous value.
    //
    // For example:
    //
    //  CRestoreValue<tstring> cResetName(m_sName);
    //  m_sName = .... some temporary name ....;
    //  .... do some stuff ....
    //  return; // m_sName will be reset to the original name.
    //
    CRestoreValue(T& cVariable)
        : m_cVariable(cVariable)
        , m_cPrevValue(cVariable) // store the previous value of the variable.
    {
        // do not modify the variable.
    }

    // the scope we were constructed in has ended, causing us to destruct.
    // Therefore, set the variable back to its original value.
    ~CRestoreValue()
    {
        m_cVariable = m_cPrevValue;
    }
};
//=============================================================================

#endif //__C_RESTOREVALUE_H__
