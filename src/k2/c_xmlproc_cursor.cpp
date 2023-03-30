// (C)2009 S2 Games
// c_xmlproc_cursor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cursor.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_bitmapresource.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

namespace XMLCursor
{
    // <cursor>
    DECLARE_XML_PROCESSOR(cursor)
    BEGIN_XML_REGISTRATION(cursor)
        REGISTER_XML_PROCESSOR(root)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(cursor, CCursor)
        pObject->SetBitmap(g_ResourceManager.Register(K2_NEW(ctx_Resources,  CBitmapResource)(node.GetProperty(_T("bitmap"))), RES_BITMAP));

        CVec2f v2Hotspot(node.GetPropertyV2(_T("hotspot")));
        pObject->SetHotspot(CVec2i(INT_ROUND(v2Hotspot.x), INT_ROUND(v2Hotspot.y)));
    END_XML_PROCESSOR(pObject)
}
