// (C)2005 S2 Games
// c_model.h
//
//=============================================================================
#ifndef __C_MODEL__
#define __C_MODEL__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
#include "i_model.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CModelFileCallback;
class CXMLNode;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CModel
//=============================================================================
class CModel : public IResource
{
private:
    tstring             m_sModelFilename;
    IModel*             m_pModelFile;
    CModelFileCallback* m_pCallback;

public:
    K2_API ~CModel()    {}
    K2_API CModel(const tstring &sPath);

    K2_API  virtual uint            GetResType() const          { return RES_MODEL; }
    K2_API  virtual const tstring&  GetResTypeName() const      { return ResTypeName(); }
    K2_API  static const tstring&   ResTypeName()               { static tstring sTypeName(_T("{model}")); return sTypeName; }

    bool    Allocate(const tstring &sName, const tstring &sFilename, const tstring &sType, const CXMLNode &node);
    
    IModel* GetModelFile()                          { return m_pModelFile; }
    void    ProcessProperties(const CXMLNode &node) { if (m_pModelFile != NULL) m_pModelFile->ProcessProperties(node); }

    int     Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
    void    Free();

    bool    LoadNull();

    K2_API SkinHandle       GetSkinHandle(const tstring &sName) const;
    K2_API const CBBoxf&    GetBounds() const;
};
//=============================================================================
#endif //__C_MODEL__
