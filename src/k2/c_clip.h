// (C)2005 S2 Games
// c_clip.h
//
//=============================================================================
#ifndef __C_CLIP__
#define __C_CLIP__

//=============================================================================
// Headers
//=============================================================================
#include "i_resource.h"
#include "c_buffer.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IModel;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SFloatKeys
{
    float           *keys;              //will allocate num_keys keys
    int             num_keys;
};

struct SQuatKeys
{
    vec4_t          *keys;
    int             num_keys;
};


struct SByteKeys
{
    byte            *keys;
    int             num_keys;
};

struct SBoneMotion
{
    tstring         sBoneName;

    // Rotation keys (converted to a quaternion for interpolation)
    SFloatKeys      keys_pitch;
    SFloatKeys      keys_roll;
    SFloatKeys      keys_yaw;

    // Position keys
    SFloatKeys      keys_x;
    SFloatKeys      keys_y;
    SFloatKeys      keys_z;

    // Scale keys
    SFloatKeys      keys_scalex;
    SFloatKeys      keys_scaley;
    SFloatKeys      keys_scalez;

    // Visibility keys
    SByteKeys       keys_visibility;

    // Precomputed Rotation keys
    SQuatKeys       keys_quat;
};

enum keyTypes_enum
{
    MKEY_X,
    MKEY_Y,
    MKEY_Z,
    MKEY_PITCH,
    MKEY_ROLL,
    MKEY_YAW,
    MKEY_VISIBILITY,
    MKEY_SCALEX,
    MKEY_SCALEY,
    MKEY_SCALEZ,
    NUM_MKEY_TYPES
};

#pragma pack(push, 1)     //make sure there's no member padding.  these structs must match up with the file read in

struct SClipHeader
{
    int iVersion;
    int iNumMotions;
    int iNumFrames;
};

typedef struct keyBlock_s
{
    int         boneIndex;
    int         key_type;       //see MKEY_* defines above
    int         num_keys;

    // At the end, so we can save space
    byte        cNameLen;
    char        szBoneName[256];
}
keyBlock_t;

#pragma pack(pop)

const int CLIP_VERSION(2);
//=============================================================================

//=============================================================================
// CClip
//=============================================================================
class CClip : public IResource
{
private:
    CBufferStatic           m_cBuffer;
    CBufferStatic           m_cTempBuffer;
    
    int                     m_iNumFrames;

    vector<SBoneMotion>     m_vMotions;

    bool    ReadBlocks(vector<block_t> vblockList, uint uiIgnoreFlags);

    bool    ParseHeader(block_t *block, uint uiIgnoreFlags);
    bool    ParseBoneMotionBlock(block_t *block);
    void    FixClip();
    void    CalcQuatKeys();

    void    ReadAngleKeys(float *data, int num_keys, SFloatKeys *keys, float tweak);
    void    ReadAngleKeys(float *data, int num_keys, SFloatKeys *keys);
    void    ReadScaleKeys(float *data, int num_keys, SFloatKeys *keys);
    void    ReadPositionKeys(float *data, int num_keys, SFloatKeys *keys);
    void    ReadByteKeys(byte *data, int num_keys, SByteKeys *keys);

public:
    K2_API ~CClip() {}
    K2_API CClip(const tstring &sPath);

    K2_API  virtual uint            GetResType() const          { return RES_CLIP; }
    K2_API  virtual const tstring&  GetResTypeName() const      { return ResTypeName(); }
    K2_API  static const tstring&   ResTypeName()               { static tstring sTypeName(_T("{clip}")); return sTypeName; }

    int     Load(uint uiIgnoreFlags, const char *pData, uint uiSize);
    void    Free();

    int     GetNumFrames() const    { return m_iNumFrames; }
    int     GetNumMotions() const   { return int(m_vMotions.size()); }
    SBoneMotion *GetBoneMotion(size_t z)    { return &m_vMotions[z]; }
};
//=============================================================================
#endif //__C_CLIP__
