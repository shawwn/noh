// (C)2005 S2 Games
// c_k2model.h
//
//=============================================================================
#ifndef __C_K2MODEL_H__
#define __C_K2MODEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_bone.h"
#include "i_model.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CMesh;
class CK2Model;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define MAX_MODEL_CLIPS 128

enum EPoseType
{
    POSE_STANDARD,
    POSE_CHARACTER,
    POSE_VEHICLE,
    POSE_GADGET,

    NUM_POSE_TYPES
};

#pragma pack(push, 1)
typedef struct modelHeader_s
{
    int version;
    int numMeshes;
    int num_sprites;
    int num_surfs;
    int numBones;
    vec3_t bmin;
    vec3_t bmax;
} modelHeader_t;

typedef struct boneBlock_s
{
    int  parentIndex;
    matrix43_t invBase;             //invbase is stored as a 4x4 matrix in the model file
    matrix43_t base;                //base is stored as a 4x4 matrix in the model file

    // At the end, so we can save space
    byte        cNameLen;
    char        szBoneName[256];
} boneBlock_t;

struct SMeshBlock
{
    int mesh_index;
    int mode;
    int num_verts;
    vec3_t bmin;
    vec3_t bmax;
    int bonelink;

    // At the end, so we can save space
    byte cNameLen;
    byte cShaderNameLen;
    char szNameBuffer[512];
};

typedef struct blendedLinksBlock_s
{
    int mesh_index;
    int num_verts;
} blendedLinksBlock_t;

typedef struct singleLinksBlock_s
{
    int mesh_index;
    int num_verts;
} singleLinksBlock_t;

typedef struct faceBlock_s
{
    int mesh_index;
    int numFaces;
    char faceIndexSize;
} faceBlock_t;

typedef struct textureCoordsBlock_s
{
    int     mesh_index;
    int     channel;
} textureCoordsBlock_t;

typedef struct tangentBlock_s
{
    int     mesh_index;
    int     channel;
} tangentBlock_t;

typedef struct colorBlock_s
{
    int     mesh_index;
    int     channel;
} colorBlock_t;

typedef struct normalBlock_s
{
    int     mesh_index;
} normalBlock_t;

typedef struct surfBlock_s
{
    int surf_index;
    int num_planes;
    int num_points;
    int num_edges;
    int num_tris;
    vec3_t bmin;
    vec3_t bmax;
    int flags;
} surfBlock_t;

typedef struct vertexBlock_s
{
    int mesh_index;
}
vertexBlock_t;
#pragma pack(pop)

class CAnim;

typedef vector<CBone>   BoneVector;
typedef vector<CMesh>   MeshVector;

extern CAxis g_axisYaw180;
//=============================================================================

//=============================================================================
// CK2Model
//=============================================================================
class CK2Model : public IModel
{
private:
    typedef hash_map<tstring, CAnim*>       AnimsMap;
    typedef hash_map<uint, CAnim*>          AnimIndicesMap;
    typedef hash_map<tstring, uint>         BoneIndicesMap;

    int                     m_iVersion;

    tstring                 m_sAltDefaultMat;
    bool                    m_bAltDefaultMat;

    AnimsMap                m_mapAnims;
    AnimIndicesMap          m_mapAnimIndicies;

    BoneVector              m_vBones;
    BoneIndicesMap          m_mapBoneIndices;

    MeshVector              m_vMeshes;
    uivector                m_vTriSurfs;

    int                     m_iGroundPlane;         // index into sprite array of groundplane

    int*                    m_pBoneMapping;     // aligns LOD bones with the base model bones
    struct SMdlSprite*      m_pSprites;         // will allocate num_sprites sprites
    int                     m_iNumSprites;

    EPoseType               m_ePoseType;

    static void         FixModelCoord(CVec3f &v3);
    static void         FixModelMatrix(matrix43_t *mat);

    bool            LoadModel(const tstring &sFilename, CK2Model *pBaseLod, uint uiIgnoreFlags);
    bool            ReadBlocks(vector<block_t> &vBlockList, uint uiIgnoreFlags);
    bool            ParseHeader(block_t *block);
    bool            ParseBoneBlock(block_t *block);
    bool            ParseMeshBlock(block_t *block);
    bool            ParseVertexBlock(block_t *block);
    bool            ParseBlendedLinksBlock(block_t *block);
    bool            ParseBlendedLinksBlock2(block_t *block);
    bool            ParseSingleLinksBlock(block_t *block);
    bool            ParseTextureCoordBlock(block_t *block);
    bool            ParseTangentBlock(block_t *block);
    bool            ParseSignBlock(block_t *block);
    bool            ParseColorBlock(block_t *block);
    bool            ParseNormalBlock(block_t *block);
    bool            ParseFaceBlock(block_t *block);
    bool            ParseSurfBlock(block_t *block);
    bool            ParseSpriteBlock(block_t *block);

    bool            FinishModel();
    void            CreateDefaultCollisionSurface();

    ResHandle       GetResourceHandle() const;

public:
    ~CK2Model();
    CK2Model();

    bool            Load(const tstring &sFileName, uint uiIgnoreFlags);
    void            ProcessProperties(const CXMLNode &node);
    void            PostLoad();
    void            SetSeed(uint uiSeed)            {}

    int             GetVersion() const              { return m_iVersion; }
    void            SetVersion(int iVersion)        { m_iVersion = iVersion; }

    EPoseType       GetPoseType() const             { return m_ePoseType; }
    float           GetLodDistance() const          { return m_fLodDistance; }
    CK2Model*       GetBaseLod() const              { return static_cast<CK2Model *>(m_pBaseLod); }

    // Anims
    K2_API CAnim*   GetAnim(const tstring &sName);
    K2_API CAnim*   GetAnim(uint uiIndex);
    K2_API uint     GetAnimIndex(const tstring &sName);
    K2_API void     AddAnim(const tstring &sName, CAnim *pAnim);
    K2_API void     ClearAnims();
    uint            GetNumAnims() const             { return uint(m_mapAnims.size()); }

    // Bones
    K2_API CBone*   GetBone(const tstring &sName);
    K2_API CBone*   GetBone(uint uiIndex);
    uint            GetNumBones() const             { return uint(m_vBones.size()); }
    K2_API void     AddBone(CBone &newBone);
    K2_API void     BuildBoneHierarchy();

    inline CBone*           GetBoneParent(uint uiIndex);
    inline const tstring&   GetBoneName(uint uiIndex);
    inline uint             GetBoneIndex(const tstring &sName);

    inline const vector<uint>*  GetBoneChildren(uint uiIndex);

    // Meshes
    K2_API void         AddMesh(const CMesh &newMesh);
    K2_API uint         GetNumMeshes() const;
    K2_API CMesh*       GetMesh(uint uiIndex);
    K2_API CMesh*       GetMesh(const tstring &sName);
    K2_API int          GetMeshIndex(const tstring &sName);
    K2_API CMesh&       AllocMesh();

    // TriSurfs
    K2_API void         AddTriSurf(uint uiIndex);
    K2_API uint         GetNumTriSurfs() const;
    K2_API CMesh*       GetTriSurf(uint uiIndex);

    K2_API void         ComputeBounds();

    K2_API void         LoadSkinMaterials(SkinHandle hSkin);
    K2_API void         LoadAllSkinMaterials();

    K2_API uint         GetNumMaterials() const;

    int*                GetBoneMapping()            { return m_pBoneMapping; }
    int                 GetBoneMapping(uint uiBone) { return m_pBoneMapping[uiBone]; }
    CK2Model*           GetLod(uint ui)             { return static_cast<CK2Model *>(m_vLods[ui]); }
};


/*====================
  CK2Model::GetBoneParent
  ====================*/
inline
CBone*  CK2Model::GetBoneParent(uint zIndex)
{
    const CBone *pBone(GetBone(zIndex));
    if (pBone == NULL)
        return NULL;

    uint uiParentIndex(pBone->GetParentIndex());
    if (uiParentIndex == INVALID_BONE)
        return NULL;

    return &m_vBones[uiParentIndex];
}


/*====================
  CK2Model::GetBoneName
  ====================*/
inline
const tstring&  CK2Model::GetBoneName(uint zIndex)
{
    const CBone *pBone(GetBone(zIndex));
    if (pBone == NULL)
        return TSNULL;
    else
        return pBone->GetName();
}


/*====================
  CK2Model::GetBoneIndex
  ====================*/
inline
uint    CK2Model::GetBoneIndex(const tstring &sName)
{
    const CBone *pBone(GetBone(sName));
    if (pBone == NULL)
        return INVALID_BONE;
    else
        return pBone->GetIndex();
}


/*====================
  CK2Model::GetBoneChildren
  ====================*/
inline
const vector<uint>* CK2Model::GetBoneChildren(uint zIndex)
{
    if (zIndex >= m_vBones.size())
        return NULL;

    return &m_vBones[zIndex].GetChildren();
}
//=============================================================================
#endif //__C_K2MODEL_H__
