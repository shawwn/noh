// (C)2005 S2 Games
// c_xmlproc_interface.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_interface.h"
#include "c_xmlprocroot.h"
#include "c_interface.h"
#include "c_interfaceresource.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"
#include "c_uitextureregistry.h"
#include "c_texture.h"
#include "c_sample.h"
#include "c_resourcemanager.h"
//=============================================================================

// <interface>
BEGIN_XML_REGISTRATION(interface)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(interface, CInterfaceResource)
    PROFILE("CXMLProc_Interface::Process");

    // Validate properties
    bool    bSnapToParent(AtoB(node.GetProperty(_T("snaptoparent"), _T("false"))));
    int     iParentSnapAt(AtoI(node.GetProperty(_T("parentsnapat"), _T("0"))));
    bool    bSnapToGrid(AtoB(node.GetProperty(_T("snaptogrid"), _T("false"))));
    int     iGridSquares(AtoI(node.GetProperty(_T("gridsquares"), _T("0"))));
    int     iGridSnapAt(AtoI(node.GetProperty(_T("gridsnapat"), _T("0"))));

    if (bSnapToParent && iParentSnapAt == 0)
    {
        Console.Warn << _T("Parent snapping is turned on and set to snap at 0 pixels. Skipping") << newl;
        bSnapToParent = false;
    }

    if (bSnapToGrid && iGridSquares == 0)
    {
        Console.Warn << _T("Grid snapping is turned on without a grid defined. Skipping") << newl;
        bSnapToGrid = false;
    }

    if (bSnapToGrid && iGridSnapAt == 0)
    {
        Console.Warn << _T("Grid snapping is turned on and set to snap at 0 pixels. Skipping") << newl;
        bSnapToGrid = false;
    }

    // Create new interface
    CWidgetStyle style(NULL, node.GetPropertyMap());
    pObject->Allocate(style);
END_XML_PROCESSOR(pObject->GetInterface())


namespace XMLInterfaceDyanmic
{
    // <texture>
    DECLARE_XML_PROCESSOR(texture)
    BEGIN_XML_REGISTRATION(texture)
        REGISTER_XML_PROCESSOR(interface)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(texture, void)
        const tstring &sPath(node.GetProperty(_T("file")));
        uint uiTextureFlags(0);
        if (node.GetPropertyBool(_T("nocompress"), false))
            uiTextureFlags |= TEX_NO_COMPRESS;
        if (node.GetPropertyBool(_T("monoasalpha"), false))
            uiTextureFlags |= TEX_MONO_AS_ALPHA;

        ResHandle hTexture;
        UITextureRegistry.Register(sPath, uiTextureFlags, hTexture);
    END_XML_PROCESSOR_NO_CHILDREN

    // <effect>
    DECLARE_XML_PROCESSOR(effect)
    BEGIN_XML_REGISTRATION(effect)
        REGISTER_XML_PROCESSOR(interface)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(effect, void)
        const tstring &sPath(node.GetProperty(_T("file")));
        g_ResourceManager.Register(sPath, RES_EFFECT);
    END_XML_PROCESSOR_NO_CHILDREN

    // <sample>
    DECLARE_XML_PROCESSOR(sample)
    BEGIN_XML_REGISTRATION(sample)
        REGISTER_XML_PROCESSOR(interface)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(sample, void)
        const tstring &sPath(node.GetProperty(_T("file")));

        uint uiFlags(SND_2D);
        CSample *pSample(K2_NEW(ctx_Resources,  CSample)(sPath, uiFlags));
        g_ResourceManager.Register(pSample, RES_SAMPLE);
    END_XML_PROCESSOR_NO_CHILDREN
}
