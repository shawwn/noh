// (C)2005 S2 Games
// c_clip.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_clip.h"
#include "i_resourcelibrary.h"
//=============================================================================

static uint s_uiClipSize(0);

//=============================================================================
// Declarations
//=============================================================================
IResource*  AllocClip(const tstring &sPath);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
IResourceLibrary    g_ResLibClip(RES_CLIP, _T("Animation Clips"), CClip::ResTypeName(), true, AllocClip);
//=============================================================================

/*====================
  AllocClip
  ====================*/
IResource*  AllocClip(const tstring &sPath)
{
    return K2_NEW(ctx_Models,  CClip)(sPath);
}


/*====================
  CClip::CClip
  ====================*/
CClip::CClip(const tstring &sPath) :
IResource(sPath, TSNULL),
m_iNumFrames(0)
{
}


/*====================
  CClip:ParseHeader
  ====================*/
bool    CClip::ParseHeader(block_t *block, uint uiIgnoreFlags)
{
    SClipHeader *in = reinterpret_cast<SClipHeader *>(block->data);

    if (block->length < sizeof(SClipHeader))
    {
        Console.Err << _T("Bad clip header") << newl;
        return false;
    }

    if (LittleInt(in->iVersion) != CLIP_VERSION)
    {
        Console.Err << _T("Bad clip version") << newl;
        return false;
    }

    if (~uiIgnoreFlags & RES_CLIP_IGNORE_POSE)
    {
        int iNumMotions = LittleInt(in->iNumMotions);
        m_vMotions.resize(iNumMotions);
    }

    m_iNumFrames = LittleInt(in->iNumFrames);

    return true;
}


/*====================
  CClip::ReadAngleKeys
  ====================*/
void    CClip::ReadAngleKeys(float *data, int num_keys, SFloatKeys *keys, float tweak)
{
    keys->keys = reinterpret_cast<float *>(m_cTempBuffer.Lock(sizeof(float) * num_keys));

    for (int n(0); n < num_keys; ++n)
    {
        keys->keys[n] = LittleFloat(data[n]) + tweak;
        if (keys->keys[n] > 180.0f)
            keys->keys[n] -= 360;
        if (keys->keys[n] <= -180.0f)
            keys->keys[n] += 360;
    }

    keys->num_keys = num_keys;
}


/*====================
  CClip::ReadAngleKeys
  ====================*/
void    CClip::ReadAngleKeys(float *data, int num_keys, SFloatKeys *keys)
{
    keys->keys = reinterpret_cast<float *>(m_cTempBuffer.Lock(sizeof(float) * num_keys));

#if BYTE_ORDER == LITTLE_ENDIAN
    MemManager.Copy(keys->keys, data, sizeof(float) * num_keys);
#else
    for (int n = 0; n < num_keys; ++n)
        keys->keys[n] = LittleFloat(data[n]);
#endif

    keys->num_keys = num_keys;
}


/*====================
  CClip::ReadScaleKeys
  ====================*/
void    CClip::ReadScaleKeys(float *data, int num_keys, SFloatKeys *keys)
{
    keys->keys = reinterpret_cast<float *>(m_cBuffer.Lock(sizeof(float) * num_keys));

#if BYTE_ORDER == LITTLE_ENDIAN
    MemManager.Copy(keys->keys, data, sizeof(float) * num_keys);
#else
    for (int n = 0; n < num_keys; ++n)
        keys->keys[n] = LittleFloat(data[n]);
#endif

    keys->num_keys = num_keys;
}


/*====================
  CClip::ReadPositionKeys
  ====================*/
void    CClip::ReadPositionKeys(float *data, int num_keys, SFloatKeys *keys)
{
    keys->keys = reinterpret_cast<float *>(m_cBuffer.Lock(sizeof(float) * num_keys));

#if BYTE_ORDER == LITTLE_ENDIAN
    MemManager.Copy(keys->keys, data, sizeof(float) * num_keys);
#else
    for (int n = 0; n < num_keys; ++n)
        keys->keys[n] = LittleFloat(data[n]);
#endif

    keys->num_keys = num_keys;
}


/*====================
  CClip::ReadByteKeys
  ====================*/
void    CClip::ReadByteKeys(byte *data, int num_keys, SByteKeys *keys)
{
    keys->keys = reinterpret_cast<byte *>(m_cBuffer.Lock(CEIL_MULTIPLE<4>(sizeof(byte) * num_keys)));

    MemManager.Copy(keys->keys, data, num_keys);

    keys->num_keys = num_keys;
}


/*====================
  CClip::CalcQuatKeys
  ====================*/
void    CClip::CalcQuatKeys()
{
    for (uint uiBone(0); uiBone != m_vMotions.size(); ++uiBone)
    {
        SBoneMotion *pMotion(&m_vMotions[uiBone]);

        int iNumKeys(MAX(MAX(pMotion->keys_pitch.num_keys, pMotion->keys_roll.num_keys), pMotion->keys_yaw.num_keys));

        SQuatKeys &cKeys(pMotion->keys_quat);

        if (pMotion->keys_pitch.num_keys == 0 || pMotion->keys_roll.num_keys == 0 || pMotion->keys_yaw.num_keys == 0)
        {
            cKeys.keys = reinterpret_cast<vec4_t *>(m_cBuffer.Lock(sizeof(vec4_t)));
            M_SetVec4(cKeys.keys[0], 0.0f, 0.0f, 0.0f, 1.0f);
            cKeys.num_keys = 1;
            continue;
        }           

        cKeys.keys = reinterpret_cast<vec4_t *>(m_cBuffer.Lock(sizeof(vec4_t) * iNumKeys));

        if (iNumKeys == 1)
        {
            M_EulerToQuat(pMotion->keys_pitch.keys[0],
                pMotion->keys_roll.keys[0],
                pMotion->keys_yaw.keys[0],
                cKeys.keys[0]);
        }
        else
        {
            for (int n(0); n < iNumKeys; ++n)
            {
                M_EulerToQuat(pMotion->keys_pitch.keys[n % pMotion->keys_pitch.num_keys],
                    pMotion->keys_roll.keys[n % pMotion->keys_roll.num_keys],
                    pMotion->keys_yaw.keys[n % pMotion->keys_yaw.num_keys],
                    cKeys.keys[n]);
            }
        }

        cKeys.num_keys = iNumKeys;

        pMotion->keys_pitch.keys = nullptr;
        pMotion->keys_roll.keys = nullptr;
        pMotion->keys_yaw.keys = nullptr;

        pMotion->keys_pitch.num_keys = 0;
        pMotion->keys_roll.num_keys = 0;
        pMotion->keys_yaw.num_keys = 0;
    }
}


/*====================
  CClip::ParseBoneMotionBlock
  ====================*/
bool    CClip::ParseBoneMotionBlock(block_t *block)
{
    keyBlock_t *in = (keyBlock_t*)block->data;

    try
    {
        int iBoneIndex = LittleInt(in->boneIndex);
        if (iBoneIndex < 0 || iBoneIndex >= int(m_vMotions.size()))
            throw _TS("Invalid bone motion index");

        int iNumKeys = LittleInt(in->num_keys);
        SBoneMotion *motion = &m_vMotions[iBoneIndex];

        StrToTString(motion->sBoneName, in->szBoneName);

        void *keydata = (byte *)in + sizeof(keyBlock_t) - 256 + in->cNameLen + 1;

        switch (LittleInt(in->key_type))
        {
        case MKEY_X:
            ReadPositionKeys((float*)keydata, iNumKeys, &motion->keys_x);
            break;

        case MKEY_Y:
            ReadPositionKeys((float*)keydata, iNumKeys, &motion->keys_y);
            break;

        case MKEY_Z:
            ReadPositionKeys((float*)keydata, iNumKeys, &motion->keys_z);
            break;

        case MKEY_PITCH:
            ReadAngleKeys((float*)keydata, iNumKeys, &motion->keys_pitch);
            break;

        case MKEY_ROLL:
            ReadAngleKeys((float*)keydata, iNumKeys, &motion->keys_roll);
            break;

        case MKEY_YAW:
            if (iBoneIndex == 0)
                ReadAngleKeys((float*)keydata, iNumKeys, &motion->keys_yaw, 180.0f);
            else
                ReadAngleKeys((float*)keydata, iNumKeys, &motion->keys_yaw);
            break;

        case MKEY_SCALEX:
            ReadScaleKeys((float*)keydata, iNumKeys, &motion->keys_scalex);
            break;

        case MKEY_SCALEY:
            ReadScaleKeys((float*)keydata, iNumKeys, &motion->keys_scaley);
            break;

        case MKEY_SCALEZ:
            ReadScaleKeys((float*)keydata, iNumKeys, &motion->keys_scalez);
            break;

        case MKEY_VISIBILITY:
            ReadByteKeys((byte*)keydata, iNumKeys, &motion->keys_visibility);
            break;

        default:
            throw _TS("Unknown motion key type: ") + XtoA(in->key_type);
        }
    }
    catch (const tstring &sReason)
    {
        Console.Err << sReason << newl;
        return false;
    }

    return true;
}


/*====================
  CClip::FixClip
  ====================*/
void    CClip::FixClip()
{
    // For models that did not export all three scale axis, fill them in
    for (int n(0); n < int(m_vMotions.size()); ++n)
    {
        SFloatKeys *keys = &m_vMotions[n].keys_scaley;
        if (keys->keys == nullptr)
        {
            keys->keys = K2_NEW_ARRAY(ctx_Models, float, 1);
            keys->keys[0] = 1.0f;
            keys->num_keys = 1;
        }

        keys = &m_vMotions[n].keys_scalez;
        if (keys->keys == nullptr)
        {
            keys->keys = K2_NEW_ARRAY(ctx_Models, float, 1);
            keys->keys[0] = 1.0f;
            keys->num_keys = 1;
        }
    }
}


/*====================
  CClip::ReadBlocks
  ====================*/
bool    CClip::ReadBlocks(vector<block_t> vblockList, uint uiIgnoreFlags)
{
    if (strcmp(vblockList[0].name, "head") != 0)
    {
        Console.Err << _T("No header for clip file") << newl;
        return false;
    }

    if (!ParseHeader(&vblockList[0], uiIgnoreFlags))
        return false;

    uint uiBufferSize(0);
    uint uiTempBufferSize(0);

    if (~uiIgnoreFlags & RES_CLIP_IGNORE_POSE)
    {
        for (uint b(1); b < vblockList.size(); ++b)
        {
            block_t *block(&vblockList[b]);
            if (strcmp(block->name, "bmtn") != 0)
                continue;

            keyBlock_t *in = (keyBlock_t*)block->data;

            int iBoneIndex = LittleInt(in->boneIndex);
            int iNumKeys = LittleInt(in->num_keys);

            SBoneMotion *pMotion(&m_vMotions[iBoneIndex]);

            switch (LittleInt(in->key_type))
            {
                case MKEY_X:
                    uiBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_x.num_keys = iNumKeys;
                    break;
                case MKEY_Y:
                    uiBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_y.num_keys = iNumKeys;
                    break;
                case MKEY_Z:
                    uiBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_z.num_keys = iNumKeys;
                    break;
                case MKEY_PITCH:
                    uiTempBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_pitch.num_keys = iNumKeys;
                    break;
                case MKEY_ROLL:
                    uiTempBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_roll.num_keys = iNumKeys;
                    break;
                case MKEY_YAW:
                    uiTempBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_yaw.num_keys = iNumKeys;
                    break;
                case MKEY_SCALEX:
                    uiBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_scalex.num_keys = iNumKeys;
                    break;
                case MKEY_SCALEY:
                    uiBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_scaley.num_keys = iNumKeys;
                    break;
                case MKEY_SCALEZ:
                    uiBufferSize += sizeof(float) * iNumKeys;
                    pMotion->keys_scalez.num_keys = iNumKeys;
                    break;
                case MKEY_VISIBILITY:
                    uiBufferSize += CEIL_MULTIPLE<4>(sizeof(byte) * iNumKeys);
                    pMotion->keys_visibility.num_keys = iNumKeys;
                    break;
            }
        }

        for (uint uiBone(0); uiBone != m_vMotions.size(); ++uiBone)
        {
            SBoneMotion *pMotion(&m_vMotions[uiBone]);

            if (pMotion->keys_pitch.num_keys == 0 || pMotion->keys_roll.num_keys == 0 || pMotion->keys_yaw.num_keys == 0)
                uiBufferSize += sizeof(vec4_t);
            else
                uiBufferSize += sizeof(vec4_t) * MAX(MAX(pMotion->keys_pitch.num_keys, pMotion->keys_roll.num_keys), pMotion->keys_yaw.num_keys);
        }

        m_cTempBuffer.Resize(uiTempBufferSize);
        m_cBuffer.Resize(uiBufferSize);

        for (uint b(1); b < vblockList.size(); ++b)
        {
            block_t *block(&vblockList[b]);
            if (strcmp(block->name, "bmtn") == 0)
            {
                if (!ParseBoneMotionBlock(block))
                    return false;
            }
        }
    }

    FixClip();
    CalcQuatKeys();

    m_cTempBuffer.Resize(0);

    s_uiClipSize += uiBufferSize;

    return true;
}


/*====================
  CClip::Load
  ====================*/
int     CClip::Load(uint uiIgnoreFlags, const char *pData, uint uiSize)
{
    PROFILE("CClip::Load");

    try
    {   
        if (!m_sPath.empty())
            Console.Res << "Loading Clip " << SingleQuoteStr(m_sPath) << newl;
        else if (!m_sName.empty())
            Console.Res << "Loading Clip " << SingleQuoteStr(m_sName) << newl;
        else
            Console.Res << "Loading Unknown Clip" << newl;

        // Check header
        if (uiSize < 4)
            return false;
        if (pData[0] != 'C' || pData[1] != 'L' || pData[2] != 'I' || pData[3] != 'P')
            return false;

        // Build the block list
        vector<block_t> vBlockList;
        if (!FileManager.BuildBlockList(pData + 4, uiSize - 4, vBlockList))
            EX_ERROR(_T("Failed to generate block list for clip"));

        if (!ReadBlocks(vBlockList, uiIgnoreFlags))
            EX_ERROR(_T("Failed to parse block list for clip"));
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CClip::Load(") + m_sName + _TS(") - "), NO_THROW);
        return RES_LOAD_FAILED;
    }

    return 0;
}


/*====================
  CClip::Free
  ====================*/
void    CClip::Free()
{
}


CMD(PrintClipSize)
{
    Console << _T("Clip Buffers: ") << GetByteString(s_uiClipSize) << newl;
    return true;
}
