// (C)2005 S2 Games
// c_xmlproc_model.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_modelallocatorregistry.h"
#include "c_model.h"
#include "i_model.h"
#include "c_mesh.h"
#include "c_skin.h"
#include "c_anim.h"
#include "c_k2model.h"
#include "c_treemodel.h"
//=============================================================================

// <model>
DECLARE_XML_PROCESSOR(model)
BEGIN_XML_REGISTRATION(model)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(model, CModel)
    const tstring &sName(node.GetProperty(_T("name")));
    const tstring &sFilename(node.GetProperty(_T("file")));
    const tstring &sType(node.GetProperty(_T("type")));

    pObject->Allocate(sName, sFilename, sType, node);
END_XML_PROCESSOR(pObject)


// <skin>
DECLARE_XML_PROCESSOR(skin)
BEGIN_XML_REGISTRATION(skin)
    REGISTER_XML_PROCESSOR(model)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(skin, CModel)
    PROFILE("CXMLProcessor_model::Process");

    IModel *pIModel(pObject->GetModelFile());
    if (pIModel == nullptr)
        return false;

    CSkin skin(node.GetProperty(_T("name")), FileManager.GetWorkingDirectory(), pIModel);
    ProcessChildren(node, &skin);
    pIModel->AddSkin(skin);

    for (uint uiLod(0); uiLod < pIModel->GetNumLods(); ++uiLod)
    {
        CSkin skin(node.GetProperty(_T("name")), FileManager.GetWorkingDirectory(), pIModel->GetLod(uiLod));
        ProcessChildren(node, &skin);
        pIModel->GetLod(uiLod)->AddSkin(skin);
    }
END_XML_PROCESSOR_NO_CHILDREN

// <reference>
DECLARE_XML_PROCESSOR(reference)
BEGIN_XML_REGISTRATION(reference)
    REGISTER_XML_PROCESSOR(skin)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(reference, CSkin)
    if (!node.HasProperty(_T("mesh")))
        pObject->SetSkinRef(node.GetProperty(_T("material")));
    else
        pObject->SetSkinRef(node.GetProperty(_T("mesh")), node.GetProperty(_T("material")));
END_XML_PROCESSOR_NO_CHILDREN


// <anim>
DECLARE_XML_PROCESSOR(anim)
BEGIN_XML_REGISTRATION(anim)
    REGISTER_XML_PROCESSOR(model)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(anim, CModel)
    PROFILE("CXMLProcessor_anim::Process");

    if (pObject->GetModelFile() == nullptr)
        return false;
    if (pObject->GetModelFile()->GetType() != MODEL_K2)
        return false;

    CK2Model *pK2Model(static_cast<CK2Model*>(pObject->GetModelFile()));

    const tstring &sName(node.GetProperty(_T("name")));
    const tstring &sClip(node.GetProperty(_T("clip")));
    int iStartFrame(node.GetPropertyInt(_T("startframe"), 0));
    int iNumFrames(node.GetPropertyInt(_T("numframes"), 0));
    float fFps(node.GetPropertyFloat(_T("fps"), 30.0f));
    bool bLoop(node.GetPropertyBool(_T("loop"), false));
    int iLoopbackFrame(node.GetPropertyInt(_T("loopbackframe"), 0));
    int iBlendTime(node.GetPropertyInt(_T("blendtime"), 200));
    bool bLock(node.GetPropertyBool(_T("lock"), false));
    int iMinStartFrame(node.GetPropertyInt(_T("minstartframe"), 0));
    int iMaxStartFrame(node.GetPropertyInt(_T("maxstartframe"), 0));

    uint uiIgnoreFlags(0);
    if (pObject->GetIgnoreFlags() & RES_MODEL_IGNORE_POSE)
        uiIgnoreFlags |= RES_CLIP_IGNORE_POSE;

    CAnim *pAnim(K2_NEW(ctx_Models,  CAnim)(
        pK2Model,
        pK2Model->GetNumAnims(),
        sName,
        sClip,
        iStartFrame,
        iNumFrames,
        iLoopbackFrame,
        bLoop,
        fFps,
        iLoopbackFrame,
        iBlendTime,
        bLock,
        iMinStartFrame,
        iMaxStartFrame,
        uiIgnoreFlags));

    if (pAnim == nullptr)
    {
        Console.Warn << _T("Failed to create new animation") << newl;
        return false;
    }

    if (!(pObject->GetIgnoreFlags() & RES_MODEL_IGNORE_EVENTS))
        ProcessChildren(node, pAnim);

    pK2Model->AddAnim(sName, pAnim);
END_XML_PROCESSOR_NO_CHILDREN


// <frameevent>
DECLARE_XML_PROCESSOR(frameevent)
BEGIN_XML_REGISTRATION(frameevent)
    REGISTER_XML_PROCESSOR(anim)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(frameevent, CAnim)
    PROFILE("CXMLProcessor_frameevent::Process");

    int iFrame(node.GetPropertyInt(_T("frame"), -1));
    if (iFrame < 0)
    {
        Console.Warn << _T("<frameevent>: invalid frame") << newl;
        return false;
    }

    const tstring &sCmd(node.GetProperty(_T("cmd")));
    pObject->AddFrameEvent(iFrame, sCmd);
    Console.PrecacheCommand(sCmd);
END_XML_PROCESSOR_NO_CHILDREN


// <startevent>
DECLARE_XML_PROCESSOR(startevent)
BEGIN_XML_REGISTRATION(startevent)
    REGISTER_XML_PROCESSOR(anim)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(startevent, CAnim)
    PROFILE("CXMLProcessor_startevent::Process");

    const tstring &sCmd(node.GetProperty(_T("cmd")));
    pObject->AddStartEvent(sCmd);
    Console.PrecacheCommand(sCmd);
END_XML_PROCESSOR_NO_CHILDREN


// <endevent>
DECLARE_XML_PROCESSOR(endevent)
BEGIN_XML_REGISTRATION(endevent)
    REGISTER_XML_PROCESSOR(anim)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(endevent, CAnim)
    PROFILE("CXMLProcessor_endevent::Process");

    const tstring &sCmd(node.GetProperty(_T("cmd")));
    pObject->AddEndEvent(sCmd);
    Console.PrecacheCommand(sCmd);
END_XML_PROCESSOR_NO_CHILDREN
    

// <spawnevent>
DECLARE_XML_PROCESSOR(spawnevent)
BEGIN_XML_REGISTRATION(spawnevent)
    REGISTER_XML_PROCESSOR(model)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(spawnevent, CModel)
    PROFILE("CXMLProcessor_spawnevent::Process");

    if (pObject->GetIgnoreFlags() & RES_MODEL_IGNORE_EVENTS)
        return true;

    IModel *pIModel(pObject->GetModelFile());
    if (pIModel == nullptr)
        return false;

    const tstring &sCmd(node.GetProperty(_T("cmd")));
    pIModel->AddSpawnEvent(sCmd);
    Console.PrecacheCommand(sCmd);
END_XML_PROCESSOR_NO_CHILDREN


// <billboardmat>
DECLARE_XML_PROCESSOR(billboardmat)
BEGIN_XML_REGISTRATION(billboardmat)
    REGISTER_XML_PROCESSOR(model)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(billboardmat, CModel)
    IModel *pModel(pObject->GetModelFile());
    if (pModel == nullptr)
        return false;
    if (pModel->GetType() != MODEL_SPEEDTREE)
    {
        Console.Err << _T("<billboardmat> in a non speedtree model") << newl;
        return false;
    }

    CTreeModel *pTree(static_cast<CTreeModel*>(pModel));

    const tstring &sName(node.GetProperty(_T("file")));

    pTree->AddBillboardMaterial(FileManager.GetWorkingDirectory() + sName);
END_XML_PROCESSOR_NO_CHILDREN


// <branchmat>
DECLARE_XML_PROCESSOR(branchmat)
BEGIN_XML_REGISTRATION(branchmat)
    REGISTER_XML_PROCESSOR(model)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(branchmat, CModel)
    IModel *pModel(pObject->GetModelFile());
    if (pModel == nullptr)
        return false;
    if (pModel->GetType() != MODEL_SPEEDTREE)
    {
        Console.Err << _T("<branchmat> in a non speedtree model") << newl;
        return false;
    }

    CTreeModel *pTree(static_cast<CTreeModel*>(pModel));

    const tstring &sName(node.GetProperty(_T("file")));

    pTree->SetBranchMaterial(FileManager.GetWorkingDirectory() + sName);
END_XML_PROCESSOR_NO_CHILDREN


// <frondmat>
DECLARE_XML_PROCESSOR(frondmat)
BEGIN_XML_REGISTRATION(frondmat)
    REGISTER_XML_PROCESSOR(model)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(frondmat, CModel)
    IModel *pModel(pObject->GetModelFile());
    if (pModel == nullptr)
        return false;
    if (pModel->GetType() != MODEL_SPEEDTREE)
    {
        Console.Err << _T("<frondmat> in a non speedtree model") << newl;
        return false;
    }

    CTreeModel *pTree(static_cast<CTreeModel*>(pModel));

    const tstring &sName(node.GetProperty(_T("file")));

    pTree->AddFrondMaterial(FileManager.GetWorkingDirectory() + sName);
END_XML_PROCESSOR_NO_CHILDREN


// <leafmat>
DECLARE_XML_PROCESSOR(leafmat)
BEGIN_XML_REGISTRATION(leafmat)
    REGISTER_XML_PROCESSOR(model)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(leafmat, CModel)
    IModel *pModel(pObject->GetModelFile());
    if (pModel == nullptr)
        return false;
    if (pModel->GetType() != MODEL_SPEEDTREE)
    {
        Console.Err << _T("<leafmat> in a non speedtree model") << newl;
        return false;
    }

    CTreeModel *pTree(static_cast<CTreeModel*>(pModel));

    const tstring &sName(node.GetProperty(_T("file")));

    pTree->AddLeafMaterial(FileManager.GetWorkingDirectory() + sName);
END_XML_PROCESSOR_NO_CHILDREN
