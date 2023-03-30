// (C)2005 S2 Games
// c_k2model.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "../public/mdlsprite_t.h"
#include "../public/blendedlink_t.h"

#include "c_k2model.h"

#include "c_anim.h"
#include "c_bone.h"
#include "c_mesh.h"
#include "c_skin.h"
#include "i_modelallocator.h"
#include "c_convexpolyhedron.h"
#include "c_model.h"
#include "c_xmlnode.h"
#include "c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
DEFINE_MODEL_ALLOCATOR(CK2Model, K2);

CAxis g_axisYaw180(0.0f, 0.0f, 180.0f);

CVAR_INT    (geom_maxBoneWeights,   4);
CVAR_BOOLF  (geom_simpleSurfaces,   false,  CONEL_DEV);
CVAR_FLOATF (geom_defaultBoxScale,  1.0f,   CONEL_DEV);

const int CURRENT_MODEL_VERSION(3);
//=============================================================================

/*====================
  CK2Model::CK2Model
  ====================*/
CK2Model::CK2Model() :
IModel(MODEL_K2),
m_iVersion(0),
m_iGroundPlane(0),
m_pBoneMapping(NULL),
m_pSprites(NULL),
m_iNumSprites(0),
m_bAltDefaultMat(0),
m_sAltDefaultMat(_T("")),
m_ePoseType(POSE_STANDARD)
{
}


/*====================
  CK2Model::CK2Model
  ====================*/
CK2Model::~CK2Model()
{
    ClearAnims();

    SAFE_DELETE_ARRAY(m_pBoneMapping);

    for (vector<CMesh>::iterator it(m_vMeshes.begin()); it != m_vMeshes.end(); ++it)
        it->Free();
}


/*====================
  CK2Model::Load
  ====================*/
bool    CK2Model::Load(const tstring &sFilename, uint uiIgnoreFlags)
{
    PROFILE("CK2Model::Load");

    bool bGood(LoadModel(sFilename, NULL, uiIgnoreFlags));

    // Load Lods
    if (bGood)
    {
        if (m_fLodDistance > 0.0f)
        {
            int i(1);
            CK2Model *pLod(NULL);

            do
            {
                tstring sLodFilename(Filename_AppendSuffix(sFilename, _T("_") + XtoA(i)));

                if (FileManager.Exists(sLodFilename))
                {
                    pLod = K2_NEW(ctx_Models,  CK2Model)();
                    pLod->LoadModel(sLodFilename, this, uiIgnoreFlags);
                    m_vLods.push_back(pLod);

                    ++i;
                }
                else
                    break;
            } while (pLod);
        }

        return true;
    }

    return false;
}


/*====================
  CK2Model::ProcessProperties
  ====================*/
void    CK2Model::ProcessProperties(const CXMLNode &node)
{
    tstring sPose(node.GetProperty(_T("pose")));
    if (CompareNoCase(sPose, _T("character")) == 0)
        m_ePoseType = POSE_CHARACTER;
    else if (CompareNoCase(sPose, _T("vehicle")) == 0)
        m_ePoseType = POSE_VEHICLE;
    else if (CompareNoCase(sPose, _T("gadget")) == 0)
        m_ePoseType = POSE_GADGET;

    if(ICvar::GetBool(node.GetProperty(_T("altmatcvar"))))
    {
        m_sAltDefaultMat = node.GetProperty(_T("altmat"));
        m_bAltDefaultMat = 1;
    }

    m_fLodDistance = 0.0f; //node.GetPropertyFloat("loddistance", 0.0f);
}


/*====================
  CK2Model::PostLoad
  ====================*/
void    CK2Model::PostLoad()
{
    for (vector<CMesh>::iterator it(m_vMeshes.begin()); it != m_vMeshes.end(); ++it)
        it->PostLoad();
}


/*====================
  CK2Model::GetAnim
  ====================*/
CAnim*  CK2Model::GetAnim(const tstring &sName)
{
    AnimsMap::iterator findit = m_mapAnims.find(sName);
    if (findit == m_mapAnims.end())
        return NULL;
    else
        return findit->second;
}


/*====================
  CK2Model::GetAnim
  ====================*/
CAnim*  CK2Model::GetAnim(uint uiIndex)
{
    AnimIndicesMap::iterator findit = m_mapAnimIndicies.find(uiIndex);
    if (findit == m_mapAnimIndicies.end())
        return NULL;
    else
        return findit->second;
}


/*====================
  CK2Model::GetAnimIndex
  ====================*/
uint    CK2Model::GetAnimIndex(const tstring &sName)
{
    CAnim *pAnim(GetAnim(sName));

    if (pAnim)
        return pAnim->GetIndex();
    else
        return -1;
}


/*====================
  CK2Model::AddAnim
  ====================*/
void    CK2Model::AddAnim(const tstring &sName, CAnim *pAnim)
{
    if (pAnim == NULL)
        return;

    AnimsMap::iterator itFind(m_mapAnims.find(sName));
    if (itFind != m_mapAnims.end())
    {
        //assert(false);
        Console.Err << _T("ERROR:  Animation ") << sName << _T(" has been defined twice!  Fix this!");
        CAnim* pAnim(itFind->second);
        K2_DELETE(pAnim);
    }

    m_mapAnims[sName] = pAnim;
    m_mapAnimIndicies[pAnim->GetIndex()] = pAnim;
}


/*====================
  CK2Model::ClearAnims
  ====================*/
void    CK2Model::ClearAnims()
{
    for (AnimsMap::iterator it(m_mapAnims.begin()); it != m_mapAnims.end(); ++it)
        K2_DELETE(it->second);

    m_mapAnims.clear();
    m_mapAnimIndicies.clear();
}


/*====================
  CK2Model::GetBone
  ====================*/
CBone*  CK2Model::GetBone(const tstring &sName)
{
    BoneIndicesMap::iterator findit = m_mapBoneIndices.find(sName);
    if (findit == m_mapBoneIndices.end())
        return NULL;
    else
        return &m_vBones[findit->second];
}

CBone*  CK2Model::GetBone(uint zIndex)
{
    if (zIndex < m_vBones.size())
        return &m_vBones[zIndex];
    else
        return NULL;
}


/*====================
  CK2Model::AddBone
  ====================*/
void    CK2Model::AddBone(CBone &newBone)
{
    m_vBones.push_back(newBone);
    m_mapBoneIndices[newBone.GetName()] = int(newBone.GetIndex());
}


/*====================
  CK2Model::BuildBoneHierarchy
  ====================*/
void    CK2Model::BuildBoneHierarchy()
{
    for (vector<CBone>::iterator it(m_vBones.begin()); it != m_vBones.end(); ++it)
    {
        uint uiParentIndex = it->GetParentIndex();
        if (uiParentIndex == INVALID_BONE)
            continue;

        m_vBones[uiParentIndex].AddChild(it->GetIndex());
    }
}


/*====================
  CK2Model::AddMesh
  ====================*/
void    CK2Model::AddMesh(const CMesh &newMesh)
{
    m_vMeshes.push_back(newMesh);
}


/*====================
  CK2Model::GetNumMeshes
  ====================*/
uint    CK2Model::GetNumMeshes() const
{
    return uint(m_vMeshes.size());
}


/*====================
  CK2Model::GetMesh
  ====================*/
CMesh*  CK2Model::GetMesh(uint uiIndex)
{
    return &m_vMeshes[uiIndex];
}


/*====================
  CK2Model::GetMesh
  ====================*/
CMesh*  CK2Model::GetMesh(const tstring &sName)
{
    for (MeshVector::iterator it(m_vMeshes.begin()); it != m_vMeshes.end(); ++it)
    {
        if (it->GetName() == sName)
            return &(*it);
    }

    return NULL;
}


/*====================
  CK2Model::GetMeshIndex
  ====================*/
int     CK2Model::GetMeshIndex(const tstring &sName)
{
    for (MeshVector::iterator it = m_vMeshes.begin(); it != m_vMeshes.end(); ++it)
    {
        if (it->GetName() == sName)
            return int(it - m_vMeshes.begin());
    }

    return -1;
}


/*====================
  CK2Model::AllocMesh
  ====================*/
CMesh&  CK2Model::AllocMesh()
{
    m_vMeshes.push_back(CMesh());

    return m_vMeshes.back();
}


/*====================
  CK2Model::AddTriSurf
  ====================*/
void    CK2Model::AddTriSurf(uint uiIndex)
{
    m_vTriSurfs.push_back(uiIndex);
}


/*====================
  CK2Model::GetNumTriSurfs
  ====================*/
uint    CK2Model::GetNumTriSurfs() const
{
    return uint(m_vTriSurfs.size());
}


/*====================
  CK2Model::GetTriSurf
  ====================*/
CMesh*  CK2Model::GetTriSurf(uint uiIndex)
{
    return &m_vMeshes[m_vTriSurfs[uiIndex]];
}


/*====================
  CK2Model::ComputeBounds
  ====================*/
void    CK2Model::ComputeBounds()
{
    m_bbBounds.Clear();

    for (MeshVector::iterator it(m_vMeshes.begin()); it != m_vMeshes.end(); ++it)
    {
        M_ClearBounds(it->bmin, it->bmax);
        for (int v = 0; v < it->num_verts; ++v)
            M_AddPointToBounds(it->verts[v], it->bmin, it->bmax);

        m_bbBounds.AddBox(CBBoxf(CVec3_cast(it->bmin), CVec3_cast(it->bmax)));
    }
}


/*====================
  CK2Model::LoadSkinMaterials
  ====================*/
void    CK2Model::LoadSkinMaterials(SkinHandle hSkin)
{
    if (hSkin < 0 || hSkin > uint(m_vSkins.size()))
        return;

    // add resources referenced by LoadMaterials() as children of the model resource.
    CGraphResource cLinkChildModelResources;
    cLinkChildModelResources.LinkChildren(GetResourceHandle());

    m_vSkins[hSkin]->LoadMaterials();
}


/*====================
  CK2Model::LoadAllSkinMaterials
  ====================*/
void    CK2Model::LoadAllSkinMaterials()
{
    // add resources referenced by LoadMaterials() as children of the model resource.
    CGraphResource cLinkChildModelResources;
    cLinkChildModelResources.LinkChildren(GetResourceHandle());

    for (SkinVector::iterator it(m_vSkins.begin()); it != m_vSkins.end(); it++)
        (*it)->LoadMaterials();
}


/*====================
  CK2Model::FixModelCoord

  Rotate point 180 degrees around z axis
  ====================*/
void    CK2Model::FixModelCoord(CVec3f &v3)
{
    v3.x *= -1.0f;
    v3.y *= -1.0f;
}


/*====================
  CK2Model::FixModelMatrix
  ====================*/
void    CK2Model::FixModelMatrix(matrix43_t *mat)
{
    // Rotate yaw 180
    mat->axis[0][0] *= -1.0f;
    mat->axis[0][1] *= -1.0f;
    mat->axis[0][2] *= -1.0f;

    mat->axis[1][0] *= -1.0f;
    mat->axis[1][1] *= -1.0f;
    mat->axis[1][2] *= -1.0f;
}


/*====================
  CK2Model::ParseHeader
  ====================*/
bool    CK2Model::ParseHeader(block_t *block)
{
    modelHeader_t *in = (modelHeader_t *)block->data;

    if (block->length < sizeof(modelHeader_t))
    {
        Console.Err << _T("Bad model header") << newl;
        return false;
    }

    int ver = LittleInt(in->version);
    if (ver != CURRENT_MODEL_VERSION)
    {
        if (ver < CURRENT_MODEL_VERSION)
            Console.Warn << _T("Obsolete model file version: ") << ver;
        if (ver > CURRENT_MODEL_VERSION)
            Console.Warn << _T("Invalid model version: ") << ver;

        Console.Warn << _T(", current version is: ") << CURRENT_MODEL_VERSION << newl;
        return false;
    }

    m_iVersion = ver;

    m_iNumSprites = LittleInt(in->num_sprites);
    if (m_iNumSprites)
        m_pSprites = K2_NEW_ARRAY(ctx_Models, SMdlSprite, m_iNumSprites);

    CVec3f v3Min(LittleFloat(in->bmin[X]), LittleFloat(in->bmin[Y]), LittleFloat(in->bmin[Z]));
    CVec3f v3Max(LittleFloat(in->bmax[X]), LittleFloat(in->bmax[Y]), LittleFloat(in->bmax[Z]));

    if (v3Min.x > v3Max.x || v3Min.y > v3Max.y || v3Min.z > v3Max.z)
    {
        SetBounds(CBBoxf(FAR_AWAY, -FAR_AWAY));
    }
    else
    {
        FixModelCoord(v3Min);
        FixModelCoord(v3Max);

        CBBoxf bbBounds;
        bbBounds.AddPoint(v3Min);
        bbBounds.AddPoint(v3Max);
        SetBounds(bbBounds);
    }

    return true;
}


/*====================
  CK2Model::ParseBoneBlock
  ====================*/
bool    CK2Model::ParseBoneBlock(block_t *block)
{
    PROFILE("CK2Model::ParseBoneBlock");

    try
    {
        // Validate header
        boneBlock_t *in = (boneBlock_t*)block->data;

        // Read each bone
        for (uint uiIndex(0); (byte*)in - block->data < int(block->length); ++uiIndex)
        {
            // Initialize new bone
            tstring sBoneName;
            StrToTString(sBoneName, in->szBoneName);
            CBone newBone(uiIndex, sBoneName, LittleInt(in->parentIndex));

            for (int i(0); i < 3; ++i)
            {
                newBone.m_invBase.pos[i] = LittleFloat(in->invBase.pos[i]);
                newBone.m_invBase.axis[0][i] = LittleFloat(in->invBase.axis[0][i]);
                newBone.m_invBase.axis[1][i] = LittleFloat(in->invBase.axis[1][i]);
                newBone.m_invBase.axis[2][i] = LittleFloat(in->invBase.axis[2][i]);
            }
            FixModelMatrix(&newBone.m_invBase);

            AddBone(newBone);

            // Advance the pointer
            in = (boneBlock_t*)((char*)in + (sizeof(boneBlock_t) - 256 + in->cNameLen + 1));
        }
    }
    catch (const tstring &sReason)
    {
        Console.Err << sReason << newl;
        return false;
    }

    BuildBoneHierarchy();
    return true;
}


/*====================
  CK2Model::ParseMeshBlock
  ====================*/
bool    CK2Model::ParseMeshBlock(block_t *block)
{
    PROFILE("CK2Model::ParseMeshBlock");

    SMeshBlock *in = (SMeshBlock*)block->data;

    // Validate the block
    if (block->length != sizeof(SMeshBlock) - sizeof(char) * 512 + in->cNameLen + in->cShaderNameLen + 2)
    {
        Console.Err << _T("Bad mesh block") << newl;
        return false;
    }

    int idx = LittleInt(in->mesh_index);
    if (idx != GetNumMeshes())
    {
        Console.Err << _T("Out of order mesh") << newl;
        return false;
    }

    CMesh &newMesh(AllocMesh());
    tstring sMeshName;
    StrToTString(sMeshName, in->szNameBuffer);
    newMesh.SetName(sMeshName);

    newMesh.surfflags = 0;
    if (newMesh.GetName().find(_T("_foliage")) != tstring::npos)
        newMesh.surfflags |= SURF_FOLIAGE;

    if (newMesh.GetName().find(_T("_nohit")) != tstring::npos)
        newMesh.surfflags |= SURF_NOT_SOLID;

    newMesh.renderflags = 0;
    if (newMesh.GetName().find(_T("_invis")) != tstring::npos)
        newMesh.renderflags |= MESH_INVIS;

    tstring sDefaultShaderName;
    StrToTString(sDefaultShaderName, &in->szNameBuffer[in->cNameLen + 1]);
    if(m_bAltDefaultMat)
    {
        newMesh.SetDefaultShaderName(sDefaultShaderName + m_sAltDefaultMat);
    }
    else
    {
        newMesh.SetDefaultShaderName(sDefaultShaderName);
    }

    newMesh.mode = LittleInt(in->mode);
    newMesh.num_verts = LittleInt(in->num_verts);

    CVec3f v3Min(LittleFloat(in->bmin[X]), LittleFloat(in->bmin[Y]), LittleFloat(in->bmin[Z]));
    CVec3f v3Max(LittleFloat(in->bmax[X]), LittleFloat(in->bmax[Y]), LittleFloat(in->bmax[Z]));
    FixModelCoord(v3Min);
    FixModelCoord(v3Max);
    CBBoxf bb;
    bb.AddPoint(v3Min);
    bb.AddPoint(v3Max);
    M_CopyVec3(vec3_cast(bb.GetMin()), newMesh.bmin);
    M_CopyVec3(vec3_cast(bb.GetMax()), newMesh.bmax);

    newMesh.bonelink = LittleInt(in->bonelink);
    newMesh.SetModel(this);

    if (newMesh.GetName().find(_T("_trisurf")) != tstring::npos)
    {
        newMesh.renderflags |= MESH_INVIS;
        AddTriSurf(idx);
    }

    return true;
}


/*====================
  CK2Model::ParseVertexBlock
  ====================*/
bool    CK2Model::ParseVertexBlock(block_t *block)
{
    vertexBlock_t *in = (vertexBlock_t *)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in single link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    mesh->verts = K2_NEW_ARRAY(ctx_Models, vec3_t, mesh->num_verts);

    vec3_t *verts = (vec3_t *)(in + 1);
    for (int n = 0; n < mesh->num_verts; ++n)
    {
        // Rotate point 180 degrees around z axis by inverting both x and y
        mesh->verts[n][0] = -LittleFloat(verts[n][0]);
        mesh->verts[n][1] = -LittleFloat(verts[n][1]);
        mesh->verts[n][2] = LittleFloat(verts[n][2]);
    }

    return true;
}


typedef pair<int, float> BlendedLinkPair;

bool LinkPredIndex(const BlendedLinkPair &elem1, const BlendedLinkPair &elem2)
{
    return elem1.first < elem2.first;
}

bool LinkPredWeight(const BlendedLinkPair &elem1, const BlendedLinkPair &elem2)
{
    return elem1.second > elem2.second;
}


/*====================
  CK2Model::ParseBlendedLinksBlock
  ====================*/
bool    CK2Model::ParseBlendedLinksBlock(block_t *block)
{
    PROFILE("CK2Model::ParseBlendedLinksBlock");

    blendedLinksBlock_t *in = (blendedLinksBlock_t*)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in blended link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    if (mesh->mode != MESH_SKINNED_BLENDED)
    {
        Console.Err << _T("Invalid blended link block") << newl;
        return false;
    }

    if (LittleInt(in->num_verts) != mesh->num_verts)
    {
        Console.Err << _T("Invalid blended link block (num_verts didn't match)") << newl;
        return false;
    }

    //alloc a pool we can use for keeping the weight data so memory doesn't get fragmented into little pieces
    int poolsize = block->length - (4 * mesh->num_verts) - sizeof(blendedLinksBlock_t);
    byte *pool = mesh->linkPool = K2_NEW_ARRAY(ctx_Models, byte, poolsize);  //allocate enough memory to store weights and indexes
    int poolpos = 0;

    mesh->blendedLinks = K2_NEW_ARRAY(ctx_Models, SBlendedLink, mesh->num_verts);
    //also allocated the single links block, as we can use this as an LOD fallback
    mesh->singleLinks = K2_NEW_ARRAY(ctx_Models, singleLink_t, mesh->num_verts);

    int pos = sizeof(blendedLinksBlock_t);
    int iMaxBoneWeights(MIN<int>(geom_maxBoneWeights, 4));
    
    for (int n = 0; n < mesh->num_verts; ++n)
    {
        float fWeight(0.0f);
        
        int iNumWeights(LittleInt(*(int *)&block->data[pos]));

        pos += 4;

        float *pfWeights((float *)&block->data[pos]);
        int *piIndexes((int *)&block->data[pos + (iNumWeights << 2)]);

        // Combine duplicate weights
        for (int c(0); c < iNumWeights; ++c)
        {
            int iIndexC(piIndexes[c]);

            for (int d(c + 1); d < iNumWeights; ++d)
            {
                if (pfWeights[d] > 0.0f && piIndexes[d] == iIndexC)
                {
                    pfWeights[c] += pfWeights[d];
                    pfWeights[d] = 0.0f;
                }
            }
        }

        int iClippedWeights(MIN(iMaxBoneWeights, iNumWeights));

        mesh->blendedLinks[n].num_weights = 0;
        mesh->blendedLinks[n].weights = (float *)(&pool[poolpos]);
        poolpos += iClippedWeights * sizeof(float);
        mesh->blendedLinks[n].indexes = (uint *)(&pool[poolpos]);
        poolpos += iClippedWeights * sizeof(uint);

        // Pick the biggest weights first
        for (int c(0); c < iClippedWeights; ++c)
        {
            float fMaxWeight(0.0f);
            int iMax(-1);

            for (int w(c); w < iNumWeights; ++w)
            {
                float fTestWeight(LittleFloat(pfWeights[w]));

                if (fTestWeight > fMaxWeight)
                {
                    fMaxWeight = fTestWeight;
                    iMax = w;
                }
            }

            if (iMax == -1)
                break;

            mesh->blendedLinks[n].weights[c] = fMaxWeight;
            mesh->blendedLinks[n].indexes[c] = LittleInt(piIndexes[iMax]);
            mesh->blendedLinks[n].num_weights++;

            fWeight += fMaxWeight;

            //if (fWeight >= 1.0f)
            //  break;

            // Shift active values up the list
            if (iMax != c)
            {
                pfWeights[iMax] = pfWeights[c];
                piIndexes[iMax] = piIndexes[c];
            }
        }

        for (int w(0); w < mesh->blendedLinks[n].num_weights; ++w)
            mesh->blendedLinks[n].weights[w] /= fWeight;

        mesh->singleLinks[n] = mesh->blendedLinks[n].indexes[0];

        pos += iNumWeights * 4 * 2;

        if (poolpos > poolsize)
            K2System.Error(_TS("CK2Model::ParseBlendedLinksBlock(") + GetName() + _TS("): overflowed link pool"));
    }

    return true;
}


/*====================
  CK2Model::ParseBlendedLinksBlock2

  New blendedlink format with guaranteed sorting, max 4 weights,
  and normalization
  ====================*/
bool    CK2Model::ParseBlendedLinksBlock2(block_t *block)
{
    PROFILE("CK2Model::ParseBlendedLinksBlock2");

    blendedLinksBlock_t *in = (blendedLinksBlock_t*)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in blended link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    if (mesh->mode != MESH_SKINNED_BLENDED)
    {
        Console.Err << _T("Invalid blended link block") << newl;
        return false;
    }

    if (LittleInt(in->num_verts) != mesh->num_verts)
    {
        Console.Err << _T("Invalid blended link block (num_verts didn't match)") << newl;
        return false;
    }

    //alloc a pool we can use for keeping the weight data so memory doesn't get fragmented into little pieces
    int poolsize = block->length - (4 * mesh->num_verts) - sizeof(blendedLinksBlock_t);
    byte *pool = mesh->linkPool = K2_NEW_ARRAY(ctx_Models, byte, poolsize);  //allocate enough memory to store weights and indexes
    int poolpos = 0;

    mesh->blendedLinks = K2_NEW_ARRAY(ctx_Models, SBlendedLink, mesh->num_verts);
    mesh->singleLinks = K2_NEW_ARRAY(ctx_Models, singleLink_t, mesh->num_verts);

    int pos = sizeof(blendedLinksBlock_t);
    int iMaxBoneWeights(MIN<int>(geom_maxBoneWeights, 4));

    if (iMaxBoneWeights == 4)
    {
        for (int n = 0; n < mesh->num_verts; ++n)
        {
            int iNumWeights(LittleInt(*(int *)&block->data[pos]));

            pos += 4;

            float *pfWeights((float *)&block->data[pos]);
            int *piIndexes((int *)&block->data[pos + (iNumWeights << 2)]);

            mesh->blendedLinks[n].num_weights = 0;
            mesh->blendedLinks[n].weights = (float *)(&pool[poolpos]);
            poolpos += iNumWeights * sizeof(float);
            mesh->blendedLinks[n].indexes = (uint *)(&pool[poolpos]);
            poolpos += iNumWeights * sizeof(uint);

#if BYTE_ORDER == LITTLE_ENDIAN
            MemManager.Copy(mesh->blendedLinks[n].weights, pfWeights, sizeof(float) * iNumWeights);
            MemManager.Copy(mesh->blendedLinks[n].indexes, piIndexes, sizeof(uint) * iNumWeights);
            
#else
            for (int c(0); c < iNumWeights; ++c)
            {
                mesh->blendedLinks[n].weights[c] = LittleFloat(pfWeights[c]);
                mesh->blendedLinks[n].indexes[c] = LittleInt(piIndexes[c]);
            }
#endif
            mesh->blendedLinks[n].num_weights = iNumWeights;
            mesh->singleLinks[n] = mesh->blendedLinks[n].indexes[0];

            pos += iNumWeights * 4 * 2;

            if (poolpos > poolsize)
                K2System.Error(_TS("CK2Model::ParseBlendedLinksBlock2(") + GetName() + _TS("): overflowed link pool"));
        }
    }
    else
    {
        for (int n = 0; n < mesh->num_verts; ++n)
        {
            float fWeight(0.0f);
            
            int iNumWeights(LittleInt(*(int *)&block->data[pos]));

            pos += 4;

            float *pfWeights((float *)&block->data[pos]);
            int *piIndexes((int *)&block->data[pos + (iNumWeights << 2)]);

            int iClippedWeights(MIN(iMaxBoneWeights, iNumWeights));

            mesh->blendedLinks[n].num_weights = 0;
            mesh->blendedLinks[n].weights = (float *)(&pool[poolpos]);
            poolpos += iClippedWeights * sizeof(float);
            mesh->blendedLinks[n].indexes = (uint *)(&pool[poolpos]);
            poolpos += iClippedWeights * sizeof(uint);

            // Pick the biggest weights first
            for (int c(0); c < iClippedWeights; ++c)
            {
                mesh->blendedLinks[n].weights[c] = LittleFloat(pfWeights[c]);
                mesh->blendedLinks[n].indexes[c] = LittleInt(piIndexes[c]);
                mesh->blendedLinks[n].num_weights++;

                fWeight += mesh->blendedLinks[n].weights[c];
            }

            for (int w(0); w < mesh->blendedLinks[n].num_weights; ++w)
                mesh->blendedLinks[n].weights[w] /= fWeight;

            mesh->singleLinks[n] = mesh->blendedLinks[n].indexes[0];

            pos += iNumWeights * 4 * 2;

            if (poolpos > poolsize)
                K2System.Error(_TS("CK2Model::ParseBlendedLinksBlock2(") + GetName() + _TS("): overflowed link pool"));
        }
    }

    return true;
}


/*====================
  CK2Model::ParseSingleLinksBlock
  ====================*/
bool    CK2Model::ParseSingleLinksBlock(block_t *block)
{
    singleLinksBlock_t *in = (singleLinksBlock_t *)block->data;

    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in single link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    if (mesh->mode != MESH_SKINNED_NONBLENDED)
    {
        Console.Err << _T("Invalid single links block") << newl;
        return false;
    }

    if (LittleInt(in->num_verts) != mesh->num_verts)
    {
        Console.Err << _T("Invalid single links block (num_verts didn't match)") << newl;
        return false;
    }

    singleLink_t *links = (singleLink_t *)(in + 1);
    mesh->singleLinks = K2_NEW_ARRAY(ctx_Models, singleLink_t, mesh->num_verts);
    for (int n = 0; n < mesh->num_verts; ++n)
        mesh->singleLinks[n] = LittleInt(links[n]);

    return true;
}


/*====================
  CK2Model::ParseTextureCoordBlock
  ====================*/
bool    CK2Model::ParseTextureCoordBlock(block_t *block)
{
    textureCoordsBlock_t *in = (textureCoordsBlock_t *)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in single link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    int iChannel(LittleInt(in->channel));
    mesh->tverts[iChannel] = K2_NEW_ARRAY(ctx_Models, vec2_t, mesh->num_verts);

    vec2_t *tverts = (vec2_t*)(in + 1);

#if BYTE_ORDER == LITTLE_ENDIAN
    MemManager.Copy(mesh->tverts[iChannel], tverts, sizeof(vec2_t) * mesh->num_verts);
#else
    for (int n = 0; n < mesh->num_verts; ++n)
    {
        mesh->tverts[iChannel][n][0] = LittleFloat(tverts[n][0]);
        mesh->tverts[iChannel][n][1] = LittleFloat(tverts[n][1]);
    }
#endif

    return true;
}


/*====================
  CK2Model::ParseTangentBlock
  ====================*/
bool    CK2Model::ParseTangentBlock(block_t *block)
{
    tangentBlock_t *in = (tangentBlock_t *)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in single link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    int iChannel(LittleInt(in->channel));
    mesh->tangents[iChannel] = K2_NEW_ARRAY(ctx_Models, vec3_t, mesh->num_verts);

    vec3_t *tangents = (vec3_t*)(in + 1);
    for (int n = 0; n < mesh->num_verts; ++n)
    {
        // Rotate point 180 degrees around z axis by inverting both x and y
        mesh->tangents[iChannel][n][0] = -LittleFloat(tangents[n][0]);
        mesh->tangents[iChannel][n][1] = -LittleFloat(tangents[n][1]);
        mesh->tangents[iChannel][n][2] = LittleFloat(tangents[n][2]);
    }

    return true;
}


/*====================
  CK2Model::ParseSignBlock
  ====================*/
bool    CK2Model::ParseSignBlock(block_t *block)
{
    tangentBlock_t *in = (tangentBlock_t *)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in single link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    int iChannel(LittleInt(in->channel));
    mesh->signs[iChannel] = K2_NEW_ARRAY(ctx_Models, byte, mesh->num_verts);

    byte *signs = (byte *)(in + 1);

    // Don't do any endian conversion since these are byte arrays
    MemManager.Copy(mesh->signs[iChannel], signs, mesh->num_verts);

    return true;
}


/*====================
  CK2Model::ParseColorBlock
  ====================*/
bool    CK2Model::ParseColorBlock(block_t *block)
{
    colorBlock_t *in = (colorBlock_t *)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in single link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    int iChannel(LittleInt(in->channel));
    mesh->colors[iChannel] = K2_NEW_ARRAY(ctx_Models, bvec4_t, mesh->num_verts);

    bvec4_t *colors = (bvec4_t *)(in + 1);

    // Don't do any endian conversion since these are byte arrays
    MemManager.Copy(mesh->colors[iChannel], colors, sizeof(bvec4_t) * mesh->num_verts);

    return true;
}


/*====================
  CK2Model::ParseNormalBlock
  ====================*/
bool    CK2Model::ParseNormalBlock(block_t *block)
{
    normalBlock_t *in = (normalBlock_t *)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in single link block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    mesh->normals = K2_NEW_ARRAY(ctx_Models, vec3_t, mesh->num_verts);

    vec3_t *normals = (vec3_t *)(in + 1);
    for (int n = 0; n < mesh->num_verts; ++n)
    {
        // Rotate point 180 degrees around z axis by inverting both x and y
        mesh->normals[n][0] = -LittleFloat(normals[n][0]);
        mesh->normals[n][1] = -LittleFloat(normals[n][1]);
        mesh->normals[n][2] = LittleFloat(normals[n][2]);
    }

    return true;
}


/*====================
  CK2Model::ParseFaceBlock
  ====================*/
bool    CK2Model::ParseFaceBlock(block_t *block)
{
    faceBlock_t *in = (faceBlock_t*)block->data;
    int idx = LittleInt(in->mesh_index);
    if (idx >= int(GetNumMeshes()) || idx < 0)
    {
        Console.Err << _T("Bad mesh index in face block") << newl;
        return false;
    }

    CMesh *mesh = GetMesh(idx);
    mesh->numFaces = LittleInt(in->numFaces);
    mesh->faceList = K2_NEW_ARRAY(ctx_Models, uivec3_t, mesh->numFaces);

    int iIndexSize(in->faceIndexSize);

    switch (iIndexSize)
    {
    case 1:
        {
            byte *faceList((byte *)(in + 1));

            for (int n = 0; n < mesh->numFaces; ++n)
            {
                for (int v = 0; v < 3; ++v)
                {
                    mesh->faceList[n][v] = *faceList;
                    ++faceList;
                }
            }
        }
        break;
    case 2:
        {
            unsigned short *faceList((unsigned short *)(in + 1));

            for (int n = 0; n < mesh->numFaces; ++n)
            {
                for (int v = 0; v < 3; ++v)
                {
                    mesh->faceList[n][v] = LittleShort(*faceList);
                    ++faceList;
                }
            }
        }
        break;
    case 4:
        {
            uint *faceList((uint *)(in + 1));

#if BYTE_ORDER == LITTLE_ENDIAN
            MemManager.Copy(mesh->faceList, faceList, sizeof(uint) * 3 * mesh->numFaces);
#else
            for (int n = 0; n < mesh->numFaces; ++n)
            {
                for (int v = 0; v < 3; ++v)
                {
                    mesh->faceList[n][v] = LittleInt(*faceList);
                    ++faceList;
                }
            }
#endif
        }
        break;
    }

    return true;
}


/*====================
 CK2Model::ParseSurfBlock
  ====================*/
bool    CK2Model::ParseSurfBlock(block_t *block)
{
    PROFILE("CK2Model::ParseSurfBlock");

    if (geom_simpleSurfaces)
        return true;

    surfBlock_t *in = (surfBlock_t*)block->data;
    
    int iNumPoints(LittleInt(in->num_points));
    int iNumEdges(LittleInt(in->num_edges));
    int iNumTris(LittleInt(in->num_tris));
    int iNumPlanes(LittleInt(in->num_planes));

    if (iNumPoints < 0 || iNumEdges < 0 || iNumTris < 0 ||
        iNumPoints * sizeof(CVec3f) > block->length ||
        iNumEdges * sizeof(CEdge) > block->length ||
        iNumTris * sizeof(uint) > block->length)
        return true;

    CVec3f v3Min, v3Max;
    for (int n(0); n < 3; ++n)
    {
        v3Min[n] = LittleFloat(in->bmin[n]);
        v3Max[n] = LittleFloat(in->bmax[n]);
    }

    CConvexPolyhedron &cSurf(AllocCollisionSurf());
    cSurf.SetBounds(CBBoxf(v3Min, v3Max));

    CPlane *planes = (CPlane *)(in + 1);
    cSurf.ReservePlanes(iNumPlanes);
    for (int n(0); n < iNumPlanes; ++n)
    {
        cSurf.AddPlane(CPlane(LittleFloat(planes[n].v3Normal.x), LittleFloat(planes[n].v3Normal.y),
            LittleFloat(planes[n].v3Normal.z), LittleFloat(planes[n].fDist)));
    }

    CVec3f *points = (CVec3f *)(planes + iNumPlanes);
    cSurf.ReservePoints(iNumPoints);
    for (int n(0); n < iNumPoints; ++n)
    {
        cSurf.AddPoint(CVec3f(LittleFloat(points[n].x), LittleFloat(points[n].y), LittleFloat(points[n].z)));
    }

    CEdge *edges = (CEdge *)(points + iNumPoints);
    cSurf.ReserveEdges(iNumEdges);
    for (int n(0); n < iNumEdges; ++n)
    {
        CEdge cNewEdge;

        cNewEdge.v3Normal.x = LittleFloat(edges[n].v3Normal.x);
        cNewEdge.v3Normal.y = LittleFloat(edges[n].v3Normal.y);
        cNewEdge.v3Normal.z = LittleFloat(edges[n].v3Normal.z);

        cNewEdge.v3Point.x = LittleFloat(edges[n].v3Point.x);
        cNewEdge.v3Point.y = LittleFloat(edges[n].v3Point.y);
        cNewEdge.v3Point.z = LittleFloat(edges[n].v3Point.z);

        cSurf.AddEdge(cNewEdge);
    }

    uint *tris = (uint *)(edges + iNumEdges);
    cSurf.ReserveTris(iNumTris);
    for (int n(0); n <  iNumTris; ++n)
    {
        cSurf.AddTri
        (
            LittleInt(tris[n * 3 + 0]),
            LittleInt(tris[n * 3 + 1]),
            LittleInt(tris[n * 3 + 2])
        );
    }

    cSurf.CalcExtents();

    return true;
}


/*====================
  CK2Model::ParseSpriteBlock
  ====================*/
bool    CK2Model::ParseSpriteBlock(block_t *block)
{
    return false;
}


/*====================
  CK2Model::ReadBlocks
  ====================*/
bool    CK2Model::ReadBlocks(vector<block_t> &vBlockList, uint uiIgnoreFlags)
{
    PROFILE("CK2Model::ReadBlocks");

    if (strcmp(vBlockList[0].name, "head") != 0)
    {
        Console.Err << _T("No header") << newl;
        return false;
    }

    if (!ParseHeader(&vBlockList[0]))
        return false;

    for (size_t b = 1; b < vBlockList.size(); ++b)
    {
        block_t *block = &vBlockList[b];
        if (strcmp(block->name, "bone") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseBoneBlock(block))
                return false;
        }
        else if (strcmp(block->name, "mesh") == 0)
        {
            if (!ParseMeshBlock(block))
                return false;
        }
        else if (strcmp(block->name, "vrts") == 0)
        {
#if 1 // Only need vrts if the server needs to do per poly tracing
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;
#endif
            if (!ParseVertexBlock(block))
                return false;
        }
        else if (strcmp(block->name, "lnk1") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseBlendedLinksBlock(block))
                return false;
        }
        else if (strcmp(block->name, "lnk2") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseSingleLinksBlock(block))
                return false;
        }
        else if (strcmp(block->name, "lnk3") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseBlendedLinksBlock2(block))
                return false;
        }
        else if (strcmp(block->name, "texc") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseTextureCoordBlock(block))
                return false;
        }
        else if (strcmp(block->name, "tang") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseTangentBlock(block))
                return false;
        }
        else if (strcmp(block->name, "sign") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseSignBlock(block))
                return false;
        }
        else if (strcmp(block->name, "colr") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseColorBlock(block))
                return false;
        }
        else if (strcmp(block->name, "nrml") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseNormalBlock(block))
                return false;
        }
        else if (strcmp(block->name, "face") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseFaceBlock(block))
                return false;
        }
        else if (strcmp(block->name, "surf") == 0)
        {
            if (!ParseSurfBlock(block))
                return false;
        }
        else if (strcmp(block->name, "sprt") == 0)
        {
            if (uiIgnoreFlags & RES_MODEL_IGNORE_GEOM)
                continue;

            if (!ParseSpriteBlock(block))
                return false;
        }
    }

    return true;
}


/*====================
  CK2Model::CreateDefaultCollisionSurface
  ====================*/
void    CK2Model::CreateDefaultCollisionSurface()
{
    CBBoxf bbModel(GetBounds());
    bbModel.ScaleXY(geom_defaultBoxScale);
    AddCollisionSurf(CConvexPolyhedron(bbModel));
}


/*====================
  CK2Model::FinishModel
  ====================*/
bool    CK2Model::FinishModel()
{
    PROFILE("CK2Model::FinishModel");

    //ComputeBounds();

    if (geom_simpleSurfaces && m_vCollisionSurfs.size() != 0)
    {
        CreateDefaultCollisionSurface();
    }
    else
    {
        // Rotate all collision surfaces by 180 degrees around the Z
        // so they match up with the rendered model.
        // (this also happens to the model itself during load)

        SurfVector &surfs(GetSurfs());
        for (SurfVector::iterator it(surfs.begin()); it != surfs.end(); ++it)
            it->Transform(V_ZERO, g_axisYaw180, 1.0f);
    }

    if (GetBaseLod())
    {
        // This model is an LOD for another model.
        // skeletal posing is always done on the base model,
        // so we need a way to map our bones to the base
        // model's bones.

        m_pBoneMapping = K2_NEW_ARRAY(ctx_Models, int, GetNumBones());

        for (uint i(0); i < GetNumBones(); ++i)
        {
            m_pBoneMapping[i] = -1;

            for (uint j(0); j < GetBaseLod()->GetNumBones(); ++j)
            {
                if (GetBoneName(i) == GetBaseLod()->GetBoneName(j))
                {
                    m_pBoneMapping[i] = int(j);
                    break;
                }
            }
        }
    }
    else
    {
        m_pBoneMapping = K2_NEW_ARRAY(ctx_Models, int, GetNumBones());

        for (uint i(0); i < GetNumBones(); ++i)
            m_pBoneMapping[i] = int(i);
    }

    return true;
}


/*====================
  CK2Model::LoadModel
  ====================*/
bool    CK2Model::LoadModel(const tstring &sFilename, CK2Model *pBaseLod, uint uiIgnoreFlags)
{
    PROFILE("CK2Model::LoadModel");

    try
    {
        // Read the file
        CFileHandle hModelFile(sFilename, FILE_READ | FILE_BINARY);
        if (!hModelFile.IsOpen())
            EX_WARN(_T("Could not open file"));

        uint uiBufferLen;
        const char *pBuffer = hModelFile.GetBuffer(uiBufferLen);
        if (pBuffer == NULL)
            EX_WARN(_T("Invalid model file"));

        // Check header
        if (uiBufferLen < 4)
            return false;
        if (pBuffer[0] != 'S' || pBuffer[1] != 'M' || pBuffer[2] != 'D' || pBuffer[3] != 'L')
            return false;

        vector<block_t> vBlockList;
        if (!FileManager.BuildBlockList(pBuffer + 4, uiBufferLen - 4, vBlockList))
            EX_WARN(_T("Failed to generate block list for model"));

        SetName(sFilename);

        if (ReadBlocks(vBlockList, uiIgnoreFlags))
        {
            m_pBaseLod = pBaseLod;

            if (FinishModel())
            {
                // Generate a default skin
                CSkin skin(_T("default"), Filename_GetPath(FileManager.SanitizePath(sFilename)), this);
                AddSkin(skin);

                // TODO: Create an anim out of the default clip

                return true;
            }
        }

        EX_WARN(_T("ReadBlocks failure"));
    }
    catch (CException &ex)
    {
        ex.Process(_TS("CK2Model::LoadModel(") + FileManager.SanitizePath(sFilename) + _TS(") - "), NO_THROW);
        return false;
    }
}


/*====================
  CK2Model::GetNumMaterials
  ====================*/
uint    CK2Model::GetNumMaterials() const
{
    return uint(m_vMeshes.size());
}


/*====================
  CK2Model::GetResourceHandle
  ====================*/
ResHandle   CK2Model::GetResourceHandle() const
{
    CModel* pModel(GetModel());
    if (pModel == NULL)
        return INVALID_RESOURCE;

    return pModel->GetHandle();
}
