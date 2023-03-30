// (C)2005 S2 Games
// c_xmlproc_material.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_console.h"
#include "stringutils.h"
#include "c_filemanager.h"
#include "c_material.h"
#include "c_materialparameter.h"
//=============================================================================

/*====================
  GetBlendMode
  ====================*/
static eBlendMode   GetBlendMode(const tstring &sBlendMode)
{
    if (TStringCompare(sBlendMode, _T("BLEND_ZERO")) == 0)
        return BLEND_ZERO;
    else if (TStringCompare(sBlendMode, _T("BLEND_ONE")) == 0)
        return BLEND_ONE;
    else if (TStringCompare(sBlendMode, _T("BLEND_SRC_COLOR")) == 0)
        return BLEND_SRC_COLOR;
    else if (TStringCompare(sBlendMode, _T("BLEND_DEST_COLOR")) == 0)
        return BLEND_DEST_COLOR;
    else if (TStringCompare(sBlendMode, _T("BLEND_ONE_MINUS_SRC_COLOR")) == 0)
        return BLEND_ONE_MINUS_SRC_COLOR;
    else if (TStringCompare(sBlendMode, _T("BLEND_ONE_MINUS_DEST_COLOR")) == 0)
        return BLEND_ONE_MINUS_DEST_COLOR;
    else if (TStringCompare(sBlendMode, _T("BLEND_SRC_ALPHA")) == 0)
        return BLEND_SRC_ALPHA;
    else if (TStringCompare(sBlendMode, _T("BLEND_DEST_ALPHA")) == 0)
        return BLEND_DEST_ALPHA;
    else if (TStringCompare(sBlendMode, _T("BLEND_ONE_MINUS_SRC_ALPHA")) == 0)
        return BLEND_ONE_MINUS_SRC_ALPHA;
    else if (TStringCompare(sBlendMode, _T("BLEND_ONE_MINUS_DEST_ALPHA")) == 0)
        return BLEND_ONE_MINUS_DEST_ALPHA;
    else
    {
        Console.Warn << _T("Unrecognized blend mode ") << SingleQuoteStr(sBlendMode) << newl;
        return BLEND_ZERO;
    }
}


/*====================
  GetCullMode
  ====================*/
static eCullMode    GetCullMode(const tstring &sCullMode)
{
    if (TStringCompare(sCullMode, _T("CULL_BACK")) == 0)
        return CULL_BACK;
    else if (TStringCompare(sCullMode, _T("CULL_FRONT")) == 0)
        return CULL_FRONT;
    else if (TStringCompare(sCullMode, _T("CULL_NONE")) == 0)
        return CULL_NONE;
    else
    {
        Console.Warn << _T("Unrecognized cull mode ") << SingleQuoteStr(sCullMode) << newl;
        return CULL_BACK;
    }
}


/*====================
  GetMaterialPhase
  ====================*/
static EMaterialPhase   GetMaterialPhase(const tstring &sMaterialPhase)
{
    if (TStringCompare(sMaterialPhase, _T("shadow")) == 0)
        return PHASE_SHADOW;
    else if (TStringCompare(sMaterialPhase, _T("color")) == 0)
        return PHASE_COLOR;
    else if (TStringCompare(sMaterialPhase, _T("depth")) == 0)
        return PHASE_DEPTH;
    else if (TStringCompare(sMaterialPhase, _T("fade")) == 0)
        return PHASE_FADE;
    else if (TStringCompare(sMaterialPhase, _T("velocity")) == 0)
        return PHASE_VELOCITY;
    else if (TStringCompare(sMaterialPhase, _T("refract")) == 0)
        return PHASE_REFRACT;
    else
    {
        Console.Warn << _T("Unrecognized material phase ") << SingleQuoteStr(sMaterialPhase) << newl;
        return EMaterialPhase(-1);
    }
}


/*====================
  GetLightingScheme
  ====================*/
static ELightingScheme  GetLightingScheme(const tstring &sLightingScheme)
{
    if (TStringCompare(sLightingScheme, _T("default")) == 0)
        return LIGHTING_DEFAULT;
    else if (TStringCompare(sLightingScheme, _T("terrain")) == 0)
        return LIGHTING_TERRAIN;
    else if (TStringCompare(sLightingScheme, _T("entity")) == 0)
        return LIGHTING_ENTITY;
    else
    {
        Console.Warn << _T("Unrecognized lighting scheme ") << SingleQuoteStr(sLightingScheme) << newl;
        return LIGHTING_DEFAULT;
    }
}


namespace XMLMaterial
{
    // <material>
    DECLARE_XML_PROCESSOR(material)
    BEGIN_XML_REGISTRATION(material)
        REGISTER_XML_PROCESSOR(root)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(material, CMaterial)
    END_XML_PROCESSOR(pObject)


    // <parameters>
    DECLARE_XML_PROCESSOR(parameters)
    BEGIN_XML_REGISTRATION(parameters)
        REGISTER_XML_PROCESSOR(material)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(parameters, CMaterial)
        pObject->SetDiffuseColor(AtoV3(node.GetProperty(_CWS("vDiffuseColor"), _CWS("1.0 1.0 1.0"))));
        pObject->SetOpacity(node.GetPropertyFloat(_CWS("fOpacity"), 1.0f));
        pObject->SetSpecularLevel(node.GetPropertyFloat(_CWS("fSpecularLevel"), 1.0f));
        pObject->SetGlossiness(node.GetPropertyFloat(_CWS("fGlossiness"), 16.0f));
        pObject->SetBumpLevel(node.GetPropertyFloat(_CWS("fBumpLevel"), 1.0f));
        pObject->SetReflect(node.GetPropertyFloat(_CWS("fReflect"), 0.0f));
        pObject->SetTreeScale(node.GetPropertyFloat(_CWS("fTreeScale"), 1.0f));
        pObject->SetLightingScheme(GetLightingScheme(node.GetProperty(_CWS("eLightingScheme"), _CWS("default"))));
    END_XML_PROCESSOR(pObject)


    // <float>
    DECLARE_XML_PROCESSOR(float)
    BEGIN_XML_REGISTRATION(float)
        REGISTER_XML_PROCESSOR(parameters)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(float, CMaterial)
        const tstring &sName(node.GetProperty(_CWS("name")));
        if (sName.empty())
            return false;

        pObject->AddParameter(K2_NEW(ctx_Resources,  CMaterialParameter)<float>(sName, node.GetPropertyFloat(_CWS("value")), node.GetPropertyFloat(_CWS("valuespeed"))));
    END_XML_PROCESSOR_NO_CHILDREN


    // <vec2>
    DECLARE_XML_PROCESSOR(vec2)
    BEGIN_XML_REGISTRATION(vec2)
        REGISTER_XML_PROCESSOR(parameters)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(vec2, CMaterial)
        const tstring &sName(node.GetProperty(_CWS("name")));
        if (sName.empty())
            return false;

        pObject->AddParameter(K2_NEW(ctx_Resources,  CMaterialParameter)<CVec2f>(sName, node.GetPropertyV2(_CWS("value")), node.GetPropertyV2(_CWS("valuespeed"))));
    END_XML_PROCESSOR_NO_CHILDREN


    // <vec3>
    DECLARE_XML_PROCESSOR(vec3)
    BEGIN_XML_REGISTRATION(vec3)
        REGISTER_XML_PROCESSOR(parameters)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(vec3, CMaterial)
        const tstring &sName(node.GetProperty(_CWS("name")));
        if (sName.empty())
            return false;

        pObject->AddParameter(K2_NEW(ctx_Resources,  CMaterialParameter)<CVec3f>(sName, node.GetPropertyV3(_CWS("value")), node.GetPropertyV3(_CWS("valuespeed"))));
    END_XML_PROCESSOR_NO_CHILDREN


    // <vec4>
    DECLARE_XML_PROCESSOR(vec4)
    BEGIN_XML_REGISTRATION(vec4)
        REGISTER_XML_PROCESSOR(parameters)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(vec4, CMaterial)
        const tstring &sName(node.GetProperty(_CWS("name")));
        if (sName.empty())
            return false;

        pObject->AddParameter(K2_NEW(ctx_Resources,  CMaterialParameter)<CVec4f>(sName, node.GetPropertyV4(_CWS("value")), node.GetPropertyV4(_CWS("valuespeed"))));
    END_XML_PROCESSOR_NO_CHILDREN


    // <phase>
    DECLARE_XML_PROCESSOR(phase)
    BEGIN_XML_REGISTRATION(phase)
        REGISTER_XML_PROCESSOR(material)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(phase, CMaterial)
        bool bTranslucent(node.GetPropertyBool(_CWS("translucent"), false));
        int iDepthBias(node.GetPropertyInt(_CWS("depthbias")));

        int iFlags(0);

        if (bTranslucent)
            iFlags |= PHASE_TRANSLUCENT;
        
        if (node.GetPropertyBool(_CWS("alphatest"), false))
            iFlags |= PHASE_ALPHA_TEST;
        
        if (node.GetPropertyBool(_CWS("depthwrite"), !bTranslucent))
            iFlags |= PHASE_DEPTH_WRITE;
        
        if (node.GetPropertyBool(_CWS("depthread"), true))
            iFlags |= PHASE_DEPTH_READ;
        
        if (node.GetPropertyBool(_CWS("depthslopebias"), iDepthBias != 0))
            iFlags |= PHASE_DEPTH_SLOPE_BIAS;

        if (!node.GetPropertyBool(_CWS("shadows"), true))
            iFlags |= PHASE_NO_SHADOWS;

        if (!node.GetPropertyBool(_CWS("lighting"), true))
            iFlags |= PHASE_NO_LIGHTING;

        if (!node.GetPropertyBool(_CWS("fog"), true))
            iFlags |= PHASE_NO_FOG;

        if (node.GetPropertyBool(_CWS("refractive"), false))
            iFlags |= PHASE_REFRACTIVE;

        if (node.GetPropertyBool(_CWS("colorwrite"), true))
            iFlags |= PHASE_COLOR_WRITE;

        if (node.GetPropertyBool(_CWS("alphawrite"), true))
            iFlags |= PHASE_ALPHA_WRITE;

        if (node.GetPropertyBool(_CWS("vampire"), false))
            iFlags |= PHASE_VAMPIRE;

        if (node.GetPropertyBool(_CWS("wireframe"), false))
            iFlags |= PHASE_WIREFRAME;

        CMaterialPhase cPhase
        (
            GetMaterialPhase(node.GetProperty(_CWS("name"))),
            node.GetProperty(_CWS("vs")),
            node.GetProperty(_CWS("ps")),
            GetBlendMode(node.GetProperty(_CWS("srcblend"), _CWS("BLEND_ONE"))),
            GetBlendMode(node.GetProperty(_CWS("dstblend"), _CWS("BLEND_ZERO"))),
            GetCullMode(node.GetProperty(_CWS("cull"))),
            node.GetPropertyInt(_CWS("depthbias"), 0),
            node.GetPropertyInt(_CWS("layer"), 0),
            node.GetPropertyFloat(_CWS("depthsortbias"), 0),
            iFlags
        );

        CMaterialPhase *pPhase(&pObject->AddPhase(cPhase));
    END_XML_PROCESSOR(pPhase)

    // <multipass>
    DECLARE_XML_PROCESSOR(multipass)
    BEGIN_XML_REGISTRATION(multipass)
        REGISTER_XML_PROCESSOR(phase)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(multipass, CMaterialPhase)
        bool bTranslucent(node.GetPropertyBool(_CWS("translucent"), false));
        int iDepthBias(node.GetPropertyInt(_CWS("depthbias")));

        int iFlags(0);

        if (bTranslucent)
            iFlags |= PHASE_TRANSLUCENT;
        
        if (node.GetPropertyBool(_CWS("alphatest"), false))
            iFlags |= PHASE_ALPHA_TEST;
        
        if (node.GetPropertyBool(_CWS("depthwrite"), !bTranslucent))
            iFlags |= PHASE_DEPTH_WRITE;
        
        if (node.GetPropertyBool(_CWS("depthread"), true))
            iFlags |= PHASE_DEPTH_READ;
        
        if (node.GetPropertyBool(_CWS("depthslopebias"), iDepthBias != 0))
            iFlags |= PHASE_DEPTH_SLOPE_BIAS;

        if (!node.GetPropertyBool(_CWS("shadows"), true))
            iFlags |= PHASE_NO_SHADOWS;

        if (!node.GetPropertyBool(_CWS("lighting"), true))
            iFlags |= PHASE_NO_LIGHTING;

        if (!node.GetPropertyBool(_CWS("fog"), true))
            iFlags |= PHASE_NO_FOG;

        if (node.GetPropertyBool(_CWS("refractive"), false))
            iFlags |= PHASE_REFRACTIVE;

        if (node.GetPropertyBool(_CWS("colorwrite"), true))
            iFlags |= PHASE_COLOR_WRITE;

        if (node.GetPropertyBool(_CWS("alphawrite"), true))
            iFlags |= PHASE_ALPHA_WRITE;

        if (node.GetPropertyBool(_CWS("vampire"), false))
            iFlags |= PHASE_VAMPIRE;

        if (node.GetPropertyBool(_CWS("wireframe"), false))
            iFlags |= PHASE_WIREFRAME;

        CMaterialPhase cPhase
        (
            EMaterialPhase(-1),
            node.GetProperty(_CWS("vs")),
            node.GetProperty(_CWS("ps")),
            GetBlendMode(node.GetProperty(_CWS("srcblend"), _CWS("BLEND_ONE"))),
            GetBlendMode(node.GetProperty(_CWS("dstblend"), _CWS("BLEND_ZERO"))),
            GetCullMode(node.GetProperty(_CWS("cull"))),
            node.GetPropertyInt(_CWS("depthbias"), 0),
            node.GetPropertyInt(_CWS("layer"), 0),
            node.GetPropertyFloat(_CWS("depthsortbias"), 0),
            iFlags
        );

        CMaterialPhase *pPhase(&pObject->AddMultiPass(cPhase));
    END_XML_PROCESSOR(pPhase)


    // <sampler>
    DECLARE_XML_PROCESSOR(sampler)
    BEGIN_XML_REGISTRATION(sampler)
        REGISTER_XML_PROCESSOR(phase)
        REGISTER_XML_PROCESSOR(multipass)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(sampler, CMaterialPhase)
        tstring sName(node.GetProperty(_CWS("name")));

        int iFlags(0);
        if (!node.GetPropertyBool(_CWS("mipmaps"), true))
            iFlags |= SAM_NO_MIPMAPS;
        if (!node.GetPropertyBool(_CWS("filtering"), true))
            iFlags |= SAM_NO_FILTERING;
        if (node.GetPropertyBool(_CWS("border"), false))
            iFlags |= SAM_BORDER;
        if (node.GetPropertyBool(_CWS("repeat"), false))
            iFlags |= SAM_REPEAT;
        if (node.GetPropertyBool(_CWS("repeat_u"), true))
            iFlags |= SAM_REPEAT_U;
        if (node.GetPropertyBool(_CWS("repeat_v"), true))
            iFlags |= SAM_REPEAT_V;
        if (node.GetPropertyBool(_CWS("normalmap"), sName.compare(0, 9, _T("normalmap")) == 0))
            iFlags |= SAM_NORMALMAP;

        int iTextureFlags(0);
        if (!node.GetPropertyBool(_CWS("mipmaps"), true))
            iTextureFlags |= TEX_NO_MIPMAPS;
        if (node.GetPropertyBool(_CWS("fullquality"), false))
            iTextureFlags |= TEX_FULL_QUALITY;
        if (node.GetPropertyBool(_CWS("nocompress"), false))
            iTextureFlags |= TEX_NO_COMPRESS;

        tstring sTextureName(node.GetProperty(_CWS("texture")));
        if (!sTextureName.empty() && sTextureName[0] != '$' && sTextureName[0] != '!')
            sTextureName = FileManager.SanitizePath(sTextureName);

        CMaterialSampler sampler
        (
            sName,
            node.GetPropertyInt(_CWS("fps"), 15),
            node.GetPropertyFloat(_CWS("offset_u"), 0.0f),
            node.GetPropertyFloat(_CWS("offset_v"), 0.0f),
            node.GetPropertyFloat(_CWS("scale_u"), 1.0f),
            node.GetPropertyFloat(_CWS("scale_v"), 1.0f),
            iFlags,
            sTextureName,
            TEXTURE_2D,
            iTextureFlags
        );

        pObject->AddSampler(sampler);
    END_XML_PROCESSOR_NO_CHILDREN


    // <samplervolume>
    DECLARE_XML_PROCESSOR(samplervolume)
    BEGIN_XML_REGISTRATION(samplervolume)
        REGISTER_XML_PROCESSOR(phase)
        REGISTER_XML_PROCESSOR(multipass)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(samplervolume, CMaterialPhase)
        int iFlags = 0;
        if (!node.GetPropertyBool(_CWS("mipmaps"), true))
            iFlags |= SAM_NO_MIPMAPS;
        if (!node.GetPropertyBool(_CWS("filtering"), true))
            iFlags |= SAM_NO_FILTERING;
        if (node.GetPropertyBool(_CWS("border"), false))
            iFlags |= SAM_BORDER;
        if (node.GetPropertyBool(_CWS("repeat"), false))
            iFlags |= SAM_REPEAT;
        if (node.GetPropertyBool(_CWS("repeat_u"), true))
            iFlags |= SAM_REPEAT_U;
        if (node.GetPropertyBool(_CWS("repeat_v"), true))
            iFlags |= SAM_REPEAT_V;
        if (node.GetPropertyBool(_CWS("repeat_w"), true))
            iFlags |= SAM_REPEAT_W;

        int iTextureFlags(0);
        if (!node.GetPropertyBool(_CWS("mipmaps"), true))
            iTextureFlags |= TEX_NO_MIPMAPS;
        if (node.GetPropertyBool(_CWS("fullquality"), false))
            iTextureFlags |= TEX_FULL_QUALITY;
        if (node.GetPropertyBool(_CWS("nocompress"), false))
            iTextureFlags |= TEX_NO_COMPRESS;

        tstring sTextureName = node.GetProperty(_CWS("texture"));
        if (!sTextureName.empty() && sTextureName[0] != '$' && sTextureName[0] != '!')
            sTextureName = FileManager.SanitizePath(sTextureName);

        CMaterialSampler sampler
        (
            node.GetProperty(_CWS("name")),
            node.GetPropertyInt(_CWS("fps"), 15),
            node.GetPropertyFloat(_CWS("offset_u"), 0.0f),
            node.GetPropertyFloat(_CWS("offset_v"), 0.0f),
            node.GetPropertyFloat(_CWS("scale_u"), 1.0f),
            node.GetPropertyFloat(_CWS("scale_v"), 1.0f),
            iFlags,
            sTextureName,
            TEXTURE_VOLUME,
            iTextureFlags
        );

        pObject->AddSampler(sampler);
    END_XML_PROCESSOR_NO_CHILDREN


    // <samplercube>
    DECLARE_XML_PROCESSOR(samplercube)
    BEGIN_XML_REGISTRATION(samplercube)
        REGISTER_XML_PROCESSOR(phase)
        REGISTER_XML_PROCESSOR(multipass)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(samplercube, CMaterialPhase)
        int iFlags(0);
        if (!node.GetPropertyBool(_CWS("mipmaps"), true))
            iFlags |= SAM_NO_MIPMAPS;
        if (!node.GetPropertyBool(_CWS("filtering"), true))
            iFlags |= SAM_NO_FILTERING;
        if (node.GetPropertyBool(_CWS("border"), false))
            iFlags |= SAM_BORDER;
        if (node.GetPropertyBool(_CWS("repeat"), false))
            iFlags |= SAM_REPEAT;
        if (node.GetPropertyBool(_CWS("repeat_u"), true))
            iFlags |= SAM_REPEAT_U;
        if (node.GetPropertyBool(_CWS("repeat_v"), true))
            iFlags |= SAM_REPEAT_V;

        int iTextureFlags(0);
        if (!node.GetPropertyBool(_CWS("mipmaps"), true))
            iTextureFlags |= TEX_NO_MIPMAPS;
        if (node.GetPropertyBool(_CWS("fullquality"), false))
            iTextureFlags |= TEX_FULL_QUALITY;
        if (node.GetPropertyBool(_CWS("nocompress"), false))
            iTextureFlags |= TEX_NO_COMPRESS;

        tstring sTextureName = node.GetProperty(_CWS("texture"));
        if (!sTextureName.empty() && sTextureName[0] != '$' && sTextureName[0] != '!')
            sTextureName = FileManager.SanitizePath(sTextureName);

        CMaterialSampler sampler
        (
            node.GetProperty(_CWS("name")),
            node.GetPropertyInt(_CWS("fps"), 15),
            node.GetPropertyFloat(_CWS("offset_u"), 0.0f),
            node.GetPropertyFloat(_CWS("offset_v"), 0.0f),
            node.GetPropertyFloat(_CWS("scale_u"), 1.0f),
            node.GetPropertyFloat(_CWS("scale_v"), 1.0f),
            iFlags,
            sTextureName,
            TEXTURE_CUBE,
            iTextureFlags
        );

        pObject->AddSampler(sampler);
    END_XML_PROCESSOR_NO_CHILDREN
}