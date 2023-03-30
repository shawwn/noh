//// (C)2005 S2 Games
//// c_Ramplist.h
////
////=============================================================================
//#ifndef __C_RampList_H__
//#define __C_RampList_H__
//
////=============================================================================
//// Headers
////=============================================================================
//#include "i_worldcomponent.h"
//#include "c_rampresource.h"
////=============================================================================
//
////=============================================================================
//// Definitions
////=============================================================================
////=============================================================================
//
////=============================================================================
//// CRampList
////=============================================================================
//class CRampList : public IWorldComponent
//{
//private:
//  typedef vector<CRampInstance>   RampList;
//
//  RampList        m_vRamps;
//
//public:
//  ~CRampList();
//  CRampList(EWorldComponent eComponent);
//
//  bool        Load(CArchive &archive, const CWorld *pWorld);
//  bool        Generate(const CWorld *pWorld);
//  bool        Serialize(IBuffer *pBuffer);
//  void        Release();
//  
//  K2_API CRampInstance    AddRamp(CRecti rRecti, ResHandle hDefinitionHandle, float fAngle);
//  K2_API bool             CheckTile(int iXC, int iYC);
//  K2_API CRecti           GetRampRect(int iXC, int iYC);
//  K2_API bool             DeleteRamp(int iXC, int iYC);
//  K2_API RampList*        GetRampList() { return &m_vRamps; }
//};
////=============================================================================
//#endif //__C_RampLIST_H__
