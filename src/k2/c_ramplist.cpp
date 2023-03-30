//// (C)2005 S2 Games
//// c_Ramplist.cpp
////
////=============================================================================
//
////=============================================================================
//// Headers
////=============================================================================
#include "k2_common.h"
//
//#include "c_ramplist.h"
//#include "c_world.h"
//#include "c_xmlmanager.h"
//#include "c_xmldoc.h"
//#include "c_resourcemanager_wrapper.h"
////=============================================================================
//
///*====================
//  CRampList::~CRampList
//  ====================*/
//CRampList::~CRampList()
//{
//  Release();
//}
//
//
///*====================
//  CRampList::CRampList
//  ====================*/
//CRampList::CRampList(EWorldComponent eComponent) :
//IWorldComponent(eComponent, _T("RampSetList"))
//{
//}
//
///*====================
//  CRampList::Load
//  ====================*/
//bool  CRampList::Load(CArchive &archive, const CWorld *pWorld)
//{
//  
//  PROFILE("CRampList::Load");
//
//  try
//  {
//      m_pWorld = pWorld;
//      if (m_pWorld == NULL)
//          EX_ERROR(_T("Invalid CWorld"));
//
//      CFileHandle hRampList(m_sName, FILE_READ | FILE_BINARY, archive);
//      if (!hRampList.IsOpen())
//          EX_ERROR(_T("No RampList found in archive"));
//
//      XMLManager.Process(hRampList, _T("Ramplist"), this);
//
//      m_bChanged = false;
//      return true;
//  }
//  catch (CException &ex)
//  {
//      ex.Process(_T("CRampList::Load() - "), NO_THROW);
//      return false;
//  }
//  
//
//  return false;
//
//}
//
//
///*====================
//  CRampList::Generate
//  ====================*/
//bool  CRampList::Generate(const CWorld *pWorld)
//{
//  try
//  {
//      Release();
//
//      m_pWorld = pWorld;
//      if (m_pWorld == NULL)
//          EX_ERROR(_T("Invalid CWorld"));
//
////        m_pRampMap = K2_NEW(ctx_World) uint[m_pWorld->GetCliffTileHeight() * m_pWorld->GetCliffTileWidth()];
////        if (m_pRampMap == NULL)
////            return false;
//      
////        MemManager.Set(m_pRampMap, 0, sizeof(uint) * (m_pWorld->GetCliffTileHeight() * m_pWorld->GetCliffTileWidth()));
//      return true;
//  }
//  catch (CException &ex)
//  {
//      ex.Process(_T("CRampList::Generate() - "), NO_THROW);
//      return false;
//  }
//}
//
//
///*====================
//  CRampList::Serialize
//  ====================*/
//bool  CRampList::Serialize(IBuffer *pBuffer)
//{
//  
//  CXMLDoc xmlRampList;
//  xmlRampList.NewNode("Ramplist");
//  for (RampList::iterator it(m_vRamps.begin()); it != m_vRamps.end(); ++it)
//  {
//      xmlRampList.NewNode("Rampinstance");
//      xmlRampList.AddProperty("angle", it->GetRampAngle());
//      xmlRampList.AddProperty("path", ResourceManager.GetPath(it->GetRampDefinition()));
//      xmlRampList.AddProperty("left", it->GetRampRect().left);
//      xmlRampList.AddProperty("right", it->GetRampRect().right);
//      xmlRampList.AddProperty("bottom", it->GetRampRect().bottom);
//      xmlRampList.AddProperty("top", it->GetRampRect().top);
//      xmlRampList.EndNode();
//  }
//  xmlRampList.EndNode();
//  pBuffer->Clear();
//  pBuffer->Write(xmlRampList.GetBuffer()->Get(), xmlRampList.GetBuffer()->GetLength());
//
//  if (pBuffer->GetFaults())
//      return false;
//  
//  return true;
//  
//}
//
//
///*====================
//  CRampList::Release
//  ====================*/
//void  CRampList::Release()
//{
//  m_pWorld = NULL;
//
//  m_vRamps.clear();
//}
//
//
///*====================
//  CRampList::AddRamp
//  ====================*/
//CRampInstance CRampList::AddRamp(CRecti rRecti, ResHandle hDefinitionHandle, float fAngle)
//{
//      for (uint i(0); i < m_vRamps.size(); i++)
//      {
//          CRecti rOverLap = m_vRamps[i].GetRampRect() & rRecti;
//          if (rOverLap.GetHeight() > 1 && rOverLap.GetWidth() > 1)
//              return m_vRamps[i];
//      }
//
//      m_vRamps.push_back(CRampInstance(hDefinitionHandle, rRecti, fAngle));
//      return m_vRamps.back();
//}
//
///*====================
//  CRampList::CheckTile
//  ====================*/
//bool  CRampList::CheckTile(int iXC, int iYC)
//{
//  for (uint i(0); i < m_vRamps.size(); i++)
//  {
//      if (m_vRamps[i].GetRampRect().Contains(CVec2i(iXC, iYC)))
//      {
//          //Rectangles are stored as verts, but we are testing tiles
//          CRecti testRect(m_vRamps[i].GetRampRect());
//          testRect.right--;
//          testRect.bottom--;
//          if (testRect.Contains(CVec2i(iXC, iYC)))
//              return true;
//      }
//  }
//
//  return false;
//}
//
///*====================
//  CRampList::GetRampRect
//  ====================*/
//CRecti      CRampList::GetRampRect(int iXC, int iYC)
//{
//  for (uint i(0); i < m_vRamps.size(); i++)
//  {
//      if (m_vRamps[i].GetRampRect().Contains(CVec2i(iXC, iYC)))
//              return m_vRamps[i].GetRampRect();
//  }
//
//  return CRecti(0,0,0,0);
//}
//
///*====================
//  CRampList::DeleteRamp
//  ====================*/
//bool    CRampList::DeleteRamp(int iXC, int iYC)
//{
//  bool bReturn(false);
//  for (uint i(0); i < m_vRamps.size(); i++)
//  {
//      if (m_vRamps[i].GetRampRect().Contains(CVec2i(iXC, iYC)))
//      {
//              m_vRamps.erase(m_vRamps.begin() + i);
//              bReturn = true;
//      }
//  }
//  return bReturn;
//}
//
