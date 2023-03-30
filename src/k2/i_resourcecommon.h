// (C)2010 S2 Games
// i_resourcecommon.h
//
//=============================================================================
#ifndef __I_RESOURCECOMMON_H__
#define __I_RESOURCECOMMON_H__

//=============================================================================
// Declarations
//=============================================================================
class IResourceLibrary;
class IResource;

class CFontMap;
class CFontFace;
class CMaterial;
class CTexture;
class CVertexShader;
class CPixelShader;
class CModel;
class CClip;
class CSample;
class CStringTable;
class CEffect;
class CInterfaceResource;
class CResourceReference;
class CStateString;
class CConvexPolyhedron;
class CPostEffect;
class CBitmapResource;
class CCursor;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const ResHandle INVALID_RESOURCE = UINT_MAX;

enum EResourceType
{
    RES_UNKNOWN,

    RES_MATERIAL2D,
    RES_MATERIAL,
    RES_VERTEX_SHADER,
    RES_PIXEL_SHADER,
    RES_TEXTURE,
    RES_RAMP,

    RES_CLIFFDEF,

    RES_MODEL,
    RES_CLIP,

    RES_FONTFACE,
    RES_FONTMAP,
    RES_SAMPLE,
    RES_STRINGTABLE,

    RES_INTERFACE,
    RES_EFFECT,
    RES_POST_EFFECT,
    RES_BITMAP,
    RES_K2CURSOR, // Damn you windows!

    RES_REFERENCE,


    NUM_RESOURCE_TYPES
};

inline EResourceType    Res_GetType(ResHandle h)    { return EResourceType((h & 0xff000000) >> 24); }
inline int              Res_GetIndex(ResHandle h)   { return (h & 0x00ffffff); }

const uint RES_EFFECT_IGNORE_ALL(BIT(0));

const uint RES_MODEL_IGNORE_VID     (BIT(0));
const uint RES_MODEL_IGNORE_EVENTS  (BIT(1));
const uint RES_MODEL_IGNORE_GEOM    (BIT(2));
const uint RES_MODEL_IGNORE_POSE    (BIT(3));

const uint RES_MODEL_SERVER         (RES_MODEL_IGNORE_VID | RES_MODEL_IGNORE_EVENTS | RES_MODEL_IGNORE_GEOM | RES_MODEL_IGNORE_POSE);

const uint RES_CLIP_IGNORE_POSE     (BIT(0));

const uint RES_LOAD_FAILED          (BIT(0));
//const uint RES_UNREGISTERED       (BIT(1));
const uint RES_EXTERNAL             (BIT(2));
const uint RES_MAP_SPECIFIC         (BIT(3));
const uint RES_FLAG_LOCALIZED       (BIT(4));

typedef vector<ResHandle>           ResourceList;
typedef vector<IResourceLibrary *>  ResourceLibVector;
typedef vector<IResource*>          ResPtrVec;
typedef map<tstring, ResHandle>     ResNameMap;

typedef IResource* (*ResRegAllocFn)(const tstring &sPath);
//=============================================================================

#endif //__I_RESOURCECOMMON_H__
